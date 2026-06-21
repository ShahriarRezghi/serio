#include <filesystem>
#include <fstream>
#include <sstream>

#include "common.h"

// ---- serialize / deserialize (in-memory) ----

TEST(API, SerializeReturnsString)
{
    auto result = Serio::serialize<Serio::Binary>({}, 42);
    EXPECT_FALSE(result.empty());
    static_assert(std::is_same_v<decltype(result), std::string>);
}

TEST(API, DeserializeFromString)
{
    std::string bytes = Serio::serialize<Serio::Binary>({}, 42);
    int v = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, v);
    EXPECT_EQ(v, 42);
}

TEST(API, DeserializeFromStringView)
{
    std::string bytes = Serio::serialize<Serio::Binary>({}, std::string("hello"));
    std::string_view sv(bytes);
    std::string out;
    Serio::deserialize<Serio::Binary>({}, sv, out);
    EXPECT_EQ(out, "hello");
}

TEST(API, DeserializeFromPointerAndLength)
{
    std::string bytes = Serio::serialize<Serio::Binary>({}, int32_t(12345));
    int32_t out = 0;
    Serio::StringView sv(bytes.data(), bytes.size());
    Serio::deserialize<Serio::Binary>({}, sv, out);
    EXPECT_EQ(out, 12345);
}

// ---- Multiple values in a single serialize call ----

TEST(API, MultipleValuesOneCall)
{
    auto bytes = Serio::serialize<Serio::Binary>({}, int(1), double(2.5), std::string("three"));
    int a;
    double b;
    std::string c;
    Serio::deserialize<Serio::Binary>({}, bytes, a, b, c);
    EXPECT_EQ(a, 1);
    EXPECT_DOUBLE_EQ(b, 2.5);
    EXPECT_EQ(c, "three");
}

TEST(API, MultipleValuesTwoArgs)
{
    int a = 42;
    std::string b = "hello";
    auto bytes = Serio::serialize<Serio::Binary>({}, a, b);
    int oa = 0;
    std::string ob;
    Serio::deserialize<Serio::Binary>({}, bytes, oa, ob);
    EXPECT_EQ(a, oa);
    EXPECT_EQ(b, ob);
}

TEST(API, MultipleValuesThreeArgs)
{
    bool flag = true;
    double val = 3.14;
    std::vector<int> vec{1, 2, 3};
    auto bytes = Serio::serialize<Serio::Binary>({}, flag, val, vec);
    bool of = false;
    double ov = 0.0;
    std::vector<int> ovec;
    Serio::deserialize<Serio::Binary>({}, bytes, of, ov, ovec);
    EXPECT_EQ(flag, of);
    EXPECT_EQ(val, ov);
    EXPECT_EQ(vec, ovec);
}

TEST(API, MultipleValuesJSON)
{
    int a = 1;
    std::string b = "json";
    bool c = false;
    auto json = Serio::serialize<Serio::JSON>({}, a, b, c);
    int oa = 0;
    std::string ob;
    bool oc = true;
    Serio::deserialize<Serio::JSON>({}, json, oa, ob, oc);
    EXPECT_EQ(a, oa);
    EXPECT_EQ(b, ob);
    EXPECT_EQ(c, oc);
}

TEST(API, MultipleValuesXML)
{
    int a = 99;
    std::string b = "xml";
    auto xml = Serio::serialize<Serio::XML>({}, a, b);
    int oa = 0;
    std::string ob;
    Serio::deserialize<Serio::XML>({}, xml, oa, ob);
    EXPECT_EQ(a, oa);
    EXPECT_EQ(b, ob);
}

// ---- save / load (file) ----

TEST(API, SaveLoadBinary)
{
    std::string path = temp_path("save_load.bin");
    std::vector<int> orig{1, 2, 3, 4, 5};
    Serio::save<Serio::Binary>({}, path, orig);
    std::vector<int> out;
    Serio::load<Serio::Binary>({}, path, out);
    EXPECT_EQ(out, orig);
    std::filesystem::remove(path);
}

TEST(API, SaveLoadJson)
{
    std::string path = temp_path("save_load.json");
    std::vector<int> orig{10, 20, 30};
    Serio::save<Serio::JSON>({}, path, orig);
    std::vector<int> out;
    Serio::load<Serio::JSON>({}, path, out);
    EXPECT_EQ(out, orig);
    std::filesystem::remove(path);
}

TEST(API, SaveLoadXml)
{
    std::string path = temp_path("save_load.xml");
    std::map<std::string, int> orig{{"a", 1}, {"b", 2}};
    Serio::save<Serio::XML>({}, path, orig);
    std::map<std::string, int> out;
    Serio::load<Serio::XML>({}, path, out);
    EXPECT_EQ(out, orig);
    std::filesystem::remove(path);
}

TEST(API, SaveLoadMultipleValues)
{
    std::string path = temp_path("multi.bin");
    int a = 42;
    std::string b = "hello";
    std::vector<double> c{1.1, 2.2};
    Serio::save<Serio::Binary>({}, path, a, b, c);
    int oa;
    std::string ob;
    std::vector<double> oc;
    Serio::load<Serio::Binary>({}, path, oa, ob, oc);
    EXPECT_EQ(oa, 42);
    EXPECT_EQ(ob, "hello");
    EXPECT_EQ(oc, c);
    std::filesystem::remove(path);
}

TEST(API, SaveLoadCustomType)
{
    std::string path = temp_path("custom.bin");
    Point2D orig{3.14f, 2.71f};
    Serio::save<Serio::Binary>({}, path, orig);
    Point2D out{};
    Serio::load<Serio::Binary>({}, path, out);
    EXPECT_EQ(out, orig);
    std::filesystem::remove(path);
}

// ---- write / read (streams) ----

TEST(API, WriteReadBinary)
{
    std::string path = temp_path("stream.bin");
    {
        std::ofstream os(path, std::ios::binary);
        Serio::write<Serio::Binary>({}, os, std::vector<int>{100, 200, 300});
    }
    {
        std::ifstream is(path, std::ios::binary);
        std::vector<int> out;
        Serio::read<Serio::Binary>({}, is, out);
        EXPECT_EQ(out, (std::vector<int>{100, 200, 300}));
    }
    std::filesystem::remove(path);
}

TEST(API, WriteReadJson)
{
    std::string path = temp_path("stream.json");
    {
        std::ofstream os(path, std::ios::binary);
        Serio::write<Serio::JSON>({}, os, std::string("hello"));
    }
    {
        std::ifstream is(path, std::ios::binary);
        std::string out;
        Serio::read<Serio::JSON>({}, is, out);
        EXPECT_EQ(out, "hello");
    }
    std::filesystem::remove(path);
}

TEST(API, WriteReadXml)
{
    std::string path = temp_path("stream.xml");
    std::pair<int, std::string> orig{1, "test"};
    {
        std::ofstream os(path, std::ios::binary);
        Serio::write<Serio::XML>({}, os, orig);
    }
    {
        std::ifstream is(path, std::ios::binary);
        std::pair<int, std::string> out;
        Serio::read<Serio::XML>({}, is, out);
        EXPECT_EQ(out, orig);
    }
    std::filesystem::remove(path);
}

TEST(API, WriteReadMultipleValues)
{
    std::string path = temp_path("multi_stream.bin");
    {
        std::ofstream os(path, std::ios::binary);
        Serio::write<Serio::Binary>({}, os, int(1), double(2.0), std::string("3"));
    }
    {
        std::ifstream is(path, std::ios::binary);
        int a;
        double b;
        std::string c;
        Serio::read<Serio::Binary>({}, is, a, b, c);
        EXPECT_EQ(a, 1);
        EXPECT_DOUBLE_EQ(b, 2.0);
        EXPECT_EQ(c, "3");
    }
    std::filesystem::remove(path);
}

TEST(API, StringStream)
{
    std::string data;
    {
        std::ostringstream oss(std::ios::binary);
        Serio::write<Serio::Binary>({}, oss, std::vector<int>{1, 2, 3});
        data = oss.str();
    }
    {
        std::istringstream iss(data, std::ios::binary);
        std::vector<int> out;
        Serio::read<Serio::Binary>({}, iss, out);
        EXPECT_EQ(out, (std::vector<int>{1, 2, 3}));
    }
}

// ---- Version macros ----

TEST(API, VersionMacrosDefined)
{
    EXPECT_GE(SERIO_VERSION_MAJOR, 0);
    EXPECT_GE(SERIO_VERSION_MINOR, 0);
    EXPECT_GE(SERIO_VERSION_PATCH, 0);
}

// ---- Format consistency ----

TEST(API, BinaryJsonProduceDifferentBytes)
{
    std::vector<int> v{1, 2, 3};
    auto bin = Serio::serialize<Serio::Binary>({}, v);
    auto json = Serio::serialize<Serio::JSON>({}, v);
    EXPECT_NE(bin, json);
}

TEST(API, JsonXmlProduceDifferentBytes)
{
    int v = 42;
    auto json = Serio::serialize<Serio::JSON>({}, v);
    auto xml = Serio::serialize<Serio::XML>({}, v);
    EXPECT_NE(json, xml);
}

TEST(API, SameBinaryOutputForSameInput)
{
    std::vector<int> v{5, 6, 7};
    auto b1 = Serio::serialize<Serio::Binary>({}, v);
    auto b2 = Serio::serialize<Serio::Binary>({}, v);
    EXPECT_EQ(b1, b2);
}

// ---- Default and brace-init options ----

TEST(API, DefaultOptionsWork)
{
    Serio::SerializeOptions sopt;
    Serio::DeserializeOptions dopt;
    auto bytes = Serio::serialize<Serio::Binary>(sopt, 99);
    int out = 0;
    Serio::deserialize<Serio::Binary>(dopt, bytes, out);
    EXPECT_EQ(out, 99);
}

TEST(API, BraceInitOptions)
{
    auto bytes = Serio::serialize<Serio::Binary>({}, std::string("test"));
    std::string out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, "test");
}
