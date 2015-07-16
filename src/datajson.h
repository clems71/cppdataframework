#pragma once

#include <array>
#include <chrono>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <data.h>
#include <json/json.h>

using JsonVal = Json::Value;

namespace black_magic
{
	// DECODERS ------------------------------------------------------------------------------------
	template<typename Policy>
	inline void jsonDecode(const JsonVal & js, std::string & obj, const Policy &) {
		obj = js.asString();
	}

	template<typename Policy>
	inline void jsonDecode(const JsonVal & js, std::chrono::seconds & obj, const Policy &) {
		const auto text = static_cast<std::string>(js.asString());
		auto digitsStr = std::string();
		auto unit = std::string();

		// Read digits
		for (size_t i=0; i<text.size(); i++) {
			if (!isdigit(text[i])) {
				digitsStr = text.substr(0, i);
				unit = text.substr(i);
				break;
			}
		}

		if (digitsStr.empty()) throw std::runtime_error("invalid std::chrono::seconds value");

		const auto value = std::stoul(digitsStr);

		switch (strHash(unit.c_str())) {
		case strHash("s"): obj = std::chrono::seconds(value); break;
		case strHash("sec"): obj = std::chrono::seconds(value); break;
		case strHash("m"): obj = std::chrono::minutes(value); break;
		case strHash("min"): obj = std::chrono::minutes(value); break;
		case strHash("h"): obj = std::chrono::hours(value); break;
		case strHash("hours"): obj = std::chrono::hours(value); break;

		default:
			throw std::runtime_error("invalid unit for std::chrono::seconds value");
			break;
		}
	}

	template<typename Policy>
	inline void jsonDecode(const JsonVal & js, std::chrono::microseconds & obj, const Policy &) {
		const auto text = static_cast<std::string>(js.asString());
		auto digitsStr = std::string();
		auto unit = std::string();

		// Read digits
		for (size_t i=0; i<text.size(); i++) {
			if (!isdigit(text[i])) {
				digitsStr = text.substr(0, i);
				unit = text.substr(i);
				break;
			}
		}

		if (digitsStr.empty()) throw std::runtime_error("invalid std::chrono::microseconds value");

		const auto value = std::stoul(digitsStr);

		switch (strHash(unit.c_str())) {
		case strHash("us"): obj = std::chrono::microseconds(value); break;
		case strHash("ms"): obj = std::chrono::milliseconds(value); break;
		case strHash("s"): obj = std::chrono::seconds(value); break;
		case strHash("sec"): obj = std::chrono::seconds(value); break;
		case strHash("m"): obj = std::chrono::minutes(value); break;
		case strHash("min"): obj = std::chrono::minutes(value); break;
		case strHash("h"): obj = std::chrono::hours(value); break;
		case strHash("hours"): obj = std::chrono::hours(value); break;

		default:
			throw std::runtime_error("invalid unit for std::chrono::microseconds value");
			break;
		}
	}

	template<typename Policy>
	inline void jsonDecode(const JsonVal & js, int & obj, const Policy &) {
		obj = static_cast<int>(js.asInt());
	}

	template<typename Policy>
	inline void jsonDecode(const JsonVal & js, size_t & obj, const Policy &) {
		obj = static_cast<size_t>(js.asInt());
	}

	template<typename Policy>
	inline void jsonDecode(const JsonVal & js, float & obj, const Policy &) {
		obj = static_cast<float>(js.asFloat());
	}

	template<typename Policy>
	inline void jsonDecode(const JsonVal & js, bool & obj, const Policy &) {
		obj = js.asBool();
	}

	// SFINAE : serializable types (data types)
	template<typename T>
	inline auto jsonDecode(const JsonVal & js, T & obj, const LazyPolicy &)
		-> typename std::enable_if<is_deserializable<T, JsonVal>::value>::type
	{
		obj.decodeLazy(js);
	}

	template<typename T>
	inline auto jsonDecode(const JsonVal & js, T & obj, const StrictPolicy &)
		-> typename std::enable_if<is_deserializable<T, JsonVal>::value>::type
	{
		obj.decodeStrict(js);
	}

	// Vector of T specialization
	template<typename T, typename Policy>
	inline void jsonDecode(const JsonVal & js, std::vector<T> & obj, const Policy & policy) {
		obj.resize(js.size());
		for (size_t i=0; i<js.size(); i++) {
			::black_magic::jsonDecode(js[int(i)], obj[i], policy);
		}
	}

	// Ptr of T specialization
	template<typename T, typename Policy>
	inline void jsonDecode(const JsonVal & js, std::shared_ptr<T> & obj, const Policy & policy) {
		if (js.type() == Json::nullValue) {
			obj.reset();
		} else {
			obj.reset(new T);
			::black_magic::jsonDecode(js, *obj, policy);
		}
	}

	// Array of char specialization
	template<size_t N, typename Policy>
	inline void jsonDecode(const JsonVal & js, std::array<char, N> & obj, const Policy &) {
		if (js.size() != obj.size()) throw std::runtime_error("invalid array size");
		for (size_t i=0; i<obj.size(); i++) {
			obj[i] = js.asString().begin()[int(i)];
		}
	}


	// ENCODERS ------------------------------------------------------------------------------------

	// Default encoder (for non serializable types)
	template<typename T>
	inline auto jsonEncode(const T & obj, JsonVal & js)
		-> typename std::enable_if<!is_serializable<T, JsonVal>::value>::type
	{
		js = obj;
	}

	// Remove ambiguity with previous function
	template<>
	inline void jsonEncode(const unsigned long & obj, JsonVal & js) {
		js = Json::UInt(obj);
	}


	template<size_t N>
	inline void jsonEncode(const std::array<char, N> & obj, JsonVal & js) {
		js = std::string(obj.begin(), obj.end());
	}

	template<>
	inline void jsonEncode(const std::chrono::seconds & obj, JsonVal & js) {
		js = std::to_string(obj.count()) + 's';
	}

	template<>
	inline void jsonEncode(const std::chrono::microseconds & obj, JsonVal & js) {
		js = std::to_string(obj.count()) + "us";
	}

	// SFINAE : serializable types (data types)
	template<class T>
	inline auto jsonEncode(const T & obj, JsonVal & js)
		-> typename std::enable_if<is_serializable<T, JsonVal>::value>::type
	{
		obj.encode(js);
	}

	// Vector of T specialization
	template<typename T>
	inline void jsonEncode(const std::vector<T> & obj, JsonVal & js) {
		js = JsonVal(Json::arrayValue);
		js.resize(obj.size());
		for (size_t i=0; i<obj.size(); i++) {
			::black_magic::jsonEncode(obj[i], js[int(i)]);
		}
	}

	// Set of T specialization
	template<typename T>
	inline void jsonEncode(const std::set<T> & obj, JsonVal & js) {
		js = JsonVal(Json::arrayValue);
		js.resize(obj.size());
		int i = 0;
		for (const auto & item : obj) {
			::black_magic::jsonEncode(item, js[i++]);
		}
	}

	// Map<str, V> specialization
	template<typename T>
	inline void jsonEncode(const std::unordered_map<std::string, T> & obj, JsonVal & js) {
		for (const auto & pair : obj) {
			::black_magic::jsonEncode(pair.second, js[pair.first]);
		}
	}

	// Ptr of T specialization : (if obj is NULL, will be null in JSON)
	template<typename T>
	inline void jsonEncode(const std::shared_ptr<T> & obj, JsonVal & js) {
		if (!obj) {
			js = JsonVal(); // Null object
		} else {
			::black_magic::jsonEncode(*obj, js);
		}
	}

} // namespace black_magic

template<>
class Encoder<JsonVal>
{
public:
	Encoder(JsonVal & js) : js_(js) {}

	template<typename T>
	inline void encodeObjectField(const char * fieldName, const T & fieldValue) {
		black_magic::jsonEncode(fieldValue, js_[fieldName]);
	}

private:
	JsonVal & js_;
};

template<>
class Decoder<JsonVal>
{
public:
	Decoder(const JsonVal & js) : js_(js) {}

	template<typename T>
	inline void decodeObjectFieldStrict(const char * fieldName, T & fieldValue) {
		black_magic::jsonDecode(js_[fieldName], fieldValue, black_magic::StrictPolicy());
	}

	template<typename T>
	inline void decodeObjectFieldLazy(const char * fieldName, T & fieldValue) {
		if (js_.isMember(fieldName)) {
			black_magic::jsonDecode(js_[fieldName], fieldValue, black_magic::LazyPolicy());
		}
	}

private:
	const JsonVal & js_;
};


// Free function to serialize free variables, not an object necessarily
template<typename T>
inline void jsonEncode(const T & obj, JsonVal & js) {
	black_magic::jsonEncode(obj, js);
}

template<typename T>
inline void jsonDecodeStrict(const JsonVal & js, T & obj) {
	black_magic::jsonDecode(js, obj, black_magic::StrictPolicy());
}

template<typename T>
inline void jsonDecodeNonStrict(const JsonVal & js, T & obj) {
	black_magic::jsonDecode(js, obj, black_magic::LazyPolicy());
}
