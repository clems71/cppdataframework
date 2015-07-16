#include <gtest/gtest.h>

#include <data.h>      // Base framework
#include <datajson.h>  // We'll perform JSON encode/decode tasks

// Sample data - used all along the tests
struct Data
{
    #define DataFields(F) \
        F(a_string, std::string, ) \
        F(an_int, size_t, 0) \
        F(a_list, std::vector<std::string>, ) \

    DATA_IMPL(DataFields)

    bool operator==(const Data & other) const {
        return memberValues() == other.memberValues();
    }

    bool operator!=(const Data & other) const {
        return !(*this == other);
    }
};

TEST(Data, data_introspection)
{
    // You don't dream, this is all compile time!
    static_assert(Data::memberIndex("a_string") == 0, "bad member index");
    static_assert(Data::memberIndex("an_int") == 1, "bad member index");
    static_assert(Data::memberIndex("a_list") == 2, "bad member index");

    // Member that does not exist => memberCount
    static_assert(Data::memberIndex("non-existent") == 3, "bad member index");

    static_assert(Data::memberCount() == 3, "bad member count");

    auto dataSource = Data {
        .a_string = "Hello world",
        .an_int = 12345u,
        .a_list = {"A", "BC", },
    };

    ASSERT_EQ(dataSource.a_string, "Hello world");
    ASSERT_EQ(dataSource.a_list.size(), 2u);

    // Access by name, type-safety still assured!!! C++14 FTW!
    std::get<Data::memberIndex("a_string")>(dataSource.memberValues()) = "Bonjour";
    std::get<Data::memberIndex("a_list")>(dataSource.memberValues()).clear();

    ASSERT_EQ(dataSource.a_string, "Bonjour");
    ASSERT_EQ(dataSource.a_list.size(), 0u);

    ASSERT_EQ(Data::memberNames()[0], std::string("a_string"));
    ASSERT_EQ(Data::memberNames()[1], std::string("an_int"));
    ASSERT_EQ(Data::memberNames()[2], std::string("a_list"));
}

TEST(Data, jsRoundTrip)
{
    const auto dataSource = Data{
        .a_string = "Hello world",
        .an_int = 12345u,
        .a_list = {"A", "BC", },
    };

    auto dataDest = Data();

    ASSERT_NE(dataSource, dataDest);

    std::string data; // serialized data

    // Encode
    {
        JsonVal js;
        dataSource.encode(js);
        data = js.toStyledString();
    }

    ASSERT_TRUE(data.size() > 0);

    // Decode
    {
        Json::Reader rdr;
        JsonVal js;
        ASSERT_TRUE(rdr.parse(data, js));
        dataDest.decodeStrict(js);
    }

    ASSERT_EQ(dataSource, dataDest);
}
