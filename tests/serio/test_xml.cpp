#include "common.h"

// ---- XML fundamental types ----

TEST(XML, Bool) { EXPECT_EQ(roundtrip_xml(true), true); }
TEST(XML, BoolFalse) { EXPECT_EQ(roundtrip_xml(false), false); }
TEST(XML, Int) { EXPECT_EQ(roundtrip_xml(42), 42); }
TEST(XML, IntNeg) { EXPECT_EQ(roundtrip_xml(-999), -999); }
TEST(XML, Int64) { EXPECT_EQ(roundtrip_xml(int64_t(-9000000000LL)), int64_t(-9000000000LL)); }
TEST(XML, Float) { EXPECT_EQ(roundtrip_xml(3.14f), 3.14f); }
TEST(XML, Double) { EXPECT_NEAR(roundtrip_xml(2.718281828), 2.718281828, 1e-6); }
TEST(XML, String) { EXPECT_EQ(roundtrip_xml(std::string("hello")), std::string("hello")); }
TEST(XML, EmptyString) { EXPECT_EQ(roundtrip_xml(std::string("")), std::string("")); }

// ---- XML containers ----

TEST(XML, VectorInt)
{
    std::vector<int> v{1, 2, 3};
    EXPECT_EQ(roundtrip_xml(v), v);
}

TEST(XML, VectorEmpty)
{
    std::vector<int> v;
    EXPECT_EQ(roundtrip_xml(v), v);
}

TEST(XML, VectorString)
{
    std::vector<std::string> v{"hello", "world"};
    EXPECT_EQ(roundtrip_xml(v), v);
}

TEST(XML, MapIntToInt)
{
    std::map<int, int> m{{1, 10}, {2, 20}};
    EXPECT_EQ(roundtrip_xml(m), m);
}

TEST(XML, SetInt)
{
    std::set<int> s{3, 1, 2};
    EXPECT_EQ(roundtrip_xml(s), s);
}

TEST(XML, Pair)
{
    std::pair<int, double> p{5, 2.5};
    EXPECT_EQ(roundtrip_xml(p), p);
}

TEST(XML, Optional)
{
    std::optional<int> o{42};
    EXPECT_EQ(roundtrip_xml(o), o);
    o = std::nullopt;
    EXPECT_EQ(roundtrip_xml(o), o);
}

// ---- XML output format ----

TEST(XML, OutputIsXml)
{
    auto xml = Serio::serialize<Serio::XML>({}, std::vector<int>{1, 2, 3});
    EXPECT_FALSE(xml.empty());
    // XML typically starts with '<'
    EXPECT_EQ(xml.front(), '<');
}

TEST(XML, OutputContainsValues)
{
    auto xml = Serio::serialize<Serio::XML>({}, std::vector<int>{42, 100});
    EXPECT_NE(xml.find("42"), std::string::npos);
    EXPECT_NE(xml.find("100"), std::string::npos);
}

// ---- binaryString in XML ----

TEST(XML, BinaryStringRoundtrip)
{
    std::string blob;
    for (int i = 0; i < 256; ++i) blob += static_cast<char>(i);
    auto xml = Serio::serialize<Serio::XML>({}, Serio::binaryString(blob));
    std::string out;
    Serio::deserialize<Serio::XML>({}, xml, Serio::binaryString(out));
    EXPECT_EQ(out, blob);
}

TEST(XML, BinaryStringEmpty)
{
    std::string blob;
    auto xml = Serio::serialize<Serio::XML>({}, Serio::binaryString(blob));
    std::string out;
    Serio::deserialize<Serio::XML>({}, xml, Serio::binaryString(out));
    EXPECT_EQ(out, blob);
}

TEST(XML, BinaryStringNoRawBytes)
{
    std::string blob("\x00\x01\x02\xFF", 4);
    auto xml = Serio::serialize<Serio::XML>({}, Serio::binaryString(blob));
    EXPECT_EQ(xml.find('\xFF'), std::string::npos);
}

// ---- NVP (named fields) in XML ----

TEST(XML, NvpConfig)
{
    Config c{false, 5, "example.com"};
    EXPECT_EQ(roundtrip_xml(c), c);
}

TEST(XML, NvpOutputHasNames)
{
    Config c{true, 3, "localhost"};
    auto xml = Serio::serialize<Serio::XML>({}, c);
    EXPECT_NE(xml.find("enabled"), std::string::npos);
    EXPECT_NE(xml.find("retries"), std::string::npos);
    EXPECT_NE(xml.find("host"), std::string::npos);
}

TEST(XML, NvpRoundtripEmpty)
{
    Config c{false, 0, ""};
    EXPECT_EQ(roundtrip_xml(c), c);
}

// ---- Custom types in XML ----

TEST(XML, CustomPoint2D)
{
    Point2D p{1.0f, 2.0f};
    EXPECT_EQ(roundtrip_xml(p), p);
}

TEST(XML, CustomVec3)
{
    Vec3 v{1.0f, 2.0f, 3.0f};
    EXPECT_EQ(roundtrip_xml(v), v);
}

TEST(XML, VectorOfCustom)
{
    std::vector<Point2D> v{{1.0f, 2.0f}, {3.0f, 4.0f}};
    EXPECT_EQ(roundtrip_xml(v), v);
}

// ---- XML shared_ptr ----

TEST(XML, SharedPtrNull)
{
    std::shared_ptr<int> p;
    auto xml = Serio::serialize<Serio::XML>({}, p);
    std::shared_ptr<int> out = std::make_shared<int>(99);
    Serio::deserialize<Serio::XML>({}, xml, out);
    EXPECT_EQ(out, nullptr);
}

TEST(XML, SharedPtrSet)
{
    auto p = std::make_shared<std::string>("test");
    auto xml = Serio::serialize<Serio::XML>({}, p);
    std::shared_ptr<std::string> out;
    Serio::deserialize<Serio::XML>({}, xml, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, "test");
}

// ---- XML compact output ----

TEST(XML, CompactOption)
{
    Serio::SerializeOptions sopt;
    sopt.compactFrom = true;
    std::vector<int> v{1, 2, 3};
    auto compact = Serio::serialize<Serio::XML>(sopt, v);
    auto pretty = Serio::serialize<Serio::XML>({}, v);
    // Compact should be ≤ pretty in length
    EXPECT_LE(compact.size(), pretty.size());
    // Round-trip
    std::vector<int> out;
    Serio::deserialize<Serio::XML>({}, compact, out);
    EXPECT_EQ(out, v);
}

// ---- Nested structures ----

TEST(XML, NestedVector)
{
    std::vector<std::vector<int>> v{{1, 2}, {3}};
    EXPECT_EQ(roundtrip_xml(v), v);
}

TEST(XML, Tuple)
{
    std::tuple<int, std::string> t{42, "hello"};
    EXPECT_EQ(roundtrip_xml(t), t);
}

TEST(XML, MapStringToVector)
{
    std::map<std::string, std::vector<int>> m{{"a", {1, 2}}, {"b", {3}}};
    EXPECT_EQ(roundtrip_xml(m), m);
}
