#pragma once

#include <array>
#include <string>
#include <tuple>
#include <vector>

// `is_serializable` type trait
// Meant to be used with std::enable_if to conditionaly use function overloads based on this trait.
template <class T, typename DataT>
struct is_serializable
{
    template <class U>
    static auto f(const U*)
        -> decltype(&U::template encode<DataT>, char(0))
    {
    }

    static std::array<char, 2> f(...) { }

    static const bool value = (sizeof(f(static_cast<T*>(nullptr))) == 1);
};

// `is_deserializable` type trait
// Meant to be used with std::enable_if to conditionaly use function overloads based on this trait.
template <class T, typename DataT>
struct is_deserializable
{
    template <class U>
    static auto f(const U*)
        -> decltype(&U::template decodeStrict<DataT>, &U::template decodeLazy<DataT>, char(0))
    {
    }

    static std::array<char, 2> f(...) { }

    static const bool value = (sizeof(f(static_cast<T*>(nullptr))) == 1);
};


// Helpers in private namespace
namespace black_magic
{
    // Helper to compute string hash at compile time
    constexpr uint32_t strHash(const char* str, uint32_t h = 0)
    {
        return !str[h] ? 5381 : (strHash(str, h+1)*33) ^ str[h];
    }

    // Will decode all fields present in struct, and raise an exception if a field is not
    // found in serialized data.
    struct StrictPolicy {};

    // Will decode all fields present in struct, and let the value intact if the field is
    // not available in serialized data.
    struct LazyPolicy {};

    template<typename T> inline T initializer(const T & t) { return t; }
    template<typename T> inline T initializer() { return T(); }

} // namespace black_magic


#define __DATA_DECL_FIELDS(name, type, ini) type name = black_magic::initializer<type>(ini);

// Serialization
#define __DATA_ENCODE_FIELD(name, type, ini) encoder.encodeObjectField(#name, name);

// Deserialization
#define __DATA_DECODE_STRICT(name, type, ini) decoder.decodeObjectFieldStrict(#name, name);
#define __DATA_DECODE_LAZY(name, type, ini) decoder.decodeObjectFieldLazy(#name, name);

#define __DATA_MEMBER_VALUE(name, type, ini) .concat(name)

#define __DATA_MEMBER_NAME(name, type, ini) #name,

#define __DATA_MEMBER_COUNT(name, type, ini) uint8_t(0),

#define __DATA_MEMBER_INDEX(name, type, ini) ++idx && black_magic::strHash(memberName) == black_magic::strHash(#name) ? idx-1 :

// Helper, when there are commas in type you set for a field
#define DATA_T(...) __VA_ARGS__

// Default impl for a encoder/decoder, invalid, need to be implemented by concrete implementations
template<typename T> struct Encoder {};
template<typename T> struct Decoder {};

template<typename TupleT>
struct TupleBuilder
{
    TupleBuilder() {}
    TupleBuilder(TupleT && tuple) : tuple_(std::move(tuple)) {}

    template<typename T>
    auto concat() {
        return TupleBuilder<decltype(std::tuple_cat(tuple_, std::make_tuple(T())))>(std::tuple_cat(tuple_, std::make_tuple(T())));
    }

    template<typename T>
    auto concat(T & val) {
        return TupleBuilder<decltype(std::tuple_cat(tuple_, std::tie(val)))>(std::tuple_cat(tuple_, std::tie(val)));
    }

    TupleT tuple() {
        return tuple_;
    }

private:
    TupleT tuple_;
};


// Implement the data type
#define DATA_IMPL(F) \
    F(__DATA_DECL_FIELDS) \
    \
public: \
    template<typename T> \
    inline void encode(T & data) const { \
        Encoder<T> encoder{data}; \
        F(__DATA_ENCODE_FIELD); \
    } \
    \
    constexpr static size_t memberIndex(const char * memberName) \
    { \
        size_t idx = 0; \
        return \
            F(__DATA_MEMBER_INDEX) \
            idx; \
    } \
    \
    constexpr static auto memberNames() { \
        const char arr[] = {F(__DATA_MEMBER_COUNT)}; \
        return std::array<const char *, sizeof(arr)> {{ F(__DATA_MEMBER_NAME) }}; \
    } \
    \
    constexpr static auto memberCount() { \
        return memberNames().size(); \
    } \
    \
    constexpr static bool hasMember(const char * memberName) \
    { \
        return memberIndex(memberName) < memberCount(); \
    } \
    \
    /* References are returned by std::tie */ \
    inline auto memberValues() { \
        return TupleBuilder<std::tuple<>>() F(__DATA_MEMBER_VALUE) .tuple(); \
    } \
    \
    inline auto memberValues() const { \
        return TupleBuilder<std::tuple<>>() F(__DATA_MEMBER_VALUE) .tuple(); \
    } \
    \
    template<typename T> \
    inline void decodeStrict(const T & data) { \
        Decoder<T> decoder{data}; \
        F(__DATA_DECODE_STRICT); \
    } \
    \
    template<typename T> \
    inline void decodeLazy(const T & data) { /* Deserialize, if field exists */ \
        Decoder<T> decoder{data}; \
        F(__DATA_DECODE_LAZY); \
    } \
