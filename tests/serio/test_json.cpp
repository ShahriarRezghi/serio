#include "common.h"

// ---- JSON fundamental types ----

TEST(JSON, Bool) { EXPECT_EQ(roundtrip_json(true), true); }
TEST(JSON, BoolFalse) { EXPECT_EQ(roundtrip_json(false), false); }
TEST(JSON, Int) { EXPECT_EQ(roundtrip_json(42), 42); }
TEST(JSON, IntNeg) { EXPECT_EQ(roundtrip_json(-100), -100); }
TEST(JSON, Int64) { EXPECT_EQ(roundtrip_json(int64_t(9000000000LL)), int64_t(9000000000LL)); }
TEST(JSON, Uint64) { EXPECT_EQ(roundtrip_json(uint64_t(18000000000000000000ULL)), uint64_t(18000000000000000000ULL)); }
TEST(JSON, Float) { EXPECT_EQ(roundtrip_json(3.14f), 3.14f); }
TEST(JSON, Double) { EXPECT_DOUBLE_EQ(roundtrip_json(2.718281828), 2.718281828); }
TEST(JSON, String) { EXPECT_EQ(roundtrip_json(std::string("hello")), std::string("hello")); }
TEST(JSON, EmptyString) { EXPECT_EQ(roundtrip_json(std::string("")), std::string("")); }

// ---- JSON containers ----

TEST(JSON, VectorInt)
{
    std::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_json(v), v);
}

TEST(JSON, VectorEmpty)
{
    std::vector<int> v;
    EXPECT_EQ(roundtrip_json(v), v);
}

TEST(JSON, VectorString)
{
    std::vector<std::string> v{"a", "b", "c"};
    EXPECT_EQ(roundtrip_json(v), v);
}

TEST(JSON, MapIntToInt)
{
    std::map<int, int> m{{1, 10}, {2, 20}};
    EXPECT_EQ(roundtrip_json(m), m);
}

TEST(JSON, MapStringToInt)
{
    std::map<std::string, int> m{{"a", 1}, {"b", 2}};
    EXPECT_EQ(roundtrip_json(m), m);
}

TEST(JSON, SetInt)
{
    std::set<int> s{1, 2, 3};
    EXPECT_EQ(roundtrip_json(s), s);
}

TEST(JSON, Pair)
{
    std::pair<int, std::string> p{1, "one"};
    EXPECT_EQ(roundtrip_json(p), p);
}

TEST(JSON, Tuple)
{
    std::tuple<int, double, std::string> t{1, 2.5, "three"};
    EXPECT_EQ(roundtrip_json(t), t);
}

TEST(JSON, Optional)
{
    std::optional<int> o{42};
    EXPECT_EQ(roundtrip_json(o), o);
    o = std::nullopt;
    EXPECT_EQ(roundtrip_json(o), o);
}

TEST(JSON, Variant)
{
    std::variant<int, std::string> v{std::string("hello")};
    EXPECT_EQ(roundtrip_json(v), v);
}

// ---- JSON custom type ----

TEST(JSON, CustomPoint2D)
{
    Point2D p{1.0f, 2.0f};
    EXPECT_EQ(roundtrip_json(p), p);
}

TEST(JSON, CustomVec3)
{
    Vec3 v{1.0f, 2.0f, 3.0f};
    EXPECT_EQ(roundtrip_json(v), v);
}

// ---- JSON output format ----

TEST(JSON, OutputIsValidJson)
{
    std::string json = Serio::serialize<Serio::JSON>({}, std::vector<int>{1, 2, 3});
    EXPECT_FALSE(json.empty());
    // Should contain brackets
    EXPECT_NE(json.find('['), std::string::npos);
}

TEST(JSON, OutputContainsValues)
{
    std::string json = Serio::serialize<Serio::JSON>({}, std::vector<int>{42, 100});
    EXPECT_NE(json.find("42"), std::string::npos);
    EXPECT_NE(json.find("100"), std::string::npos);
}

TEST(JSON, StringInJson)
{
    std::string json = Serio::serialize<Serio::JSON>({}, std::string("hello"));
    EXPECT_NE(json.find("hello"), std::string::npos);
}

// ---- binaryString in JSON (base64 encoding) ----

TEST(JSON, BinaryStringRoundtrip)
{
    std::string blob;
    for (int i = 0; i < 256; ++i) blob += static_cast<char>(i);
    auto json = Serio::serialize<Serio::JSON>({}, Serio::binaryString(blob));
    std::string out;
    Serio::deserialize<Serio::JSON>({}, json, Serio::binaryString(out));
    EXPECT_EQ(out, blob);
}

TEST(JSON, BinaryStringEmpty)
{
    std::string blob;
    auto json = Serio::serialize<Serio::JSON>({}, Serio::binaryString(blob));
    std::string out;
    Serio::deserialize<Serio::JSON>({}, json, Serio::binaryString(out));
    EXPECT_EQ(out, blob);
}

TEST(JSON, BinaryStringIsBase64)
{
    std::string blob = "\x00\x01\x02\x03\xFF";
    blob.resize(5);
    auto json = Serio::serialize<Serio::JSON>({}, Serio::binaryString(blob));
    // Should be base64, not raw bytes
    EXPECT_EQ(json.find('\x00'), std::string::npos);
    EXPECT_EQ(json.find('\xFF'), std::string::npos);
}

TEST(JSON, BinaryStringNoOpInBinary)
{
    std::string blob = "\x01\x02\x03";
    auto bin1 = Serio::serialize<Serio::Binary>({}, blob);
    auto bin2 = Serio::serialize<Serio::Binary>({}, Serio::binaryString(blob));
    EXPECT_EQ(bin1, bin2);
}

// ---- NVP (named fields) in JSON ----

TEST(JSON, NvpConfig)
{
    Config c{true, 3, "localhost"};
    EXPECT_EQ(roundtrip_json(c), c);
}

TEST(JSON, NvpOutputHasNames)
{
    Config c{true, 3, "localhost"};
    auto json = Serio::serialize<Serio::JSON>({}, c);
    EXPECT_NE(json.find("enabled"), std::string::npos);
    EXPECT_NE(json.find("retries"), std::string::npos);
    EXPECT_NE(json.find("host"), std::string::npos);
    EXPECT_NE(json.find("localhost"), std::string::npos);
}

TEST(JSON, NvpIsObject)
{
    Config c{true, 3, "localhost"};
    auto json = Serio::serialize<Serio::JSON>({}, c);
    // An object starts with '{'
    EXPECT_EQ(json.find('{'), 0u);
}

TEST(JSON, NvpRoundtripFalse)
{
    Config c{false, 0, ""};
    EXPECT_EQ(roundtrip_json(c), c);
}

// ---- Standalone nvp at top level (serialize only; deserialize via Config) ----

TEST(JSON, StandaloneNvpSerialize)
{
    int value = 42;
    auto sn = Serio::nvp("answer", value);
    auto json = Serio::serialize<Serio::JSON>({}, sn);
    EXPECT_NE(json.find("answer"), std::string::npos);
    EXPECT_NE(json.find("42"), std::string::npos);
}

// ---- Nested containers in JSON ----

TEST(JSON, NestedVector)
{
    std::vector<std::vector<int>> v{{1, 2}, {3, 4, 5}, {}};
    EXPECT_EQ(roundtrip_json(v), v);
}

TEST(JSON, VectorOfCustom)
{
    std::vector<Point2D> v{{1.0f, 2.0f}, {3.0f, 4.0f}};
    EXPECT_EQ(roundtrip_json(v), v);
}

// ---- JSON shared_ptr ----

TEST(JSON, SharedPtrNull)
{
    std::shared_ptr<int> p;
    auto json = Serio::serialize<Serio::JSON>({}, p);
    std::shared_ptr<int> out = std::make_shared<int>(99);
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out, nullptr);
}

TEST(JSON, SharedPtrSet)
{
    auto p = std::make_shared<int>(42);
    auto json = Serio::serialize<Serio::JSON>({}, p);
    std::shared_ptr<int> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 42);
}
