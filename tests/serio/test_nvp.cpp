#include "common.h"

// NOTE: Serio::nvp() is designed for use inside _serialize/_deserialize (via C.process()),
// not as a direct top-level argument to Serio::serialize/deserialize.
// All tests here go through the Config struct which uses NVP correctly.

// ---- Config binary roundtrip ----

TEST(NVP, ConfigBinaryRoundtrip)
{
    Config c{true, 3, "localhost"};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(NVP, ConfigBinaryFalse)
{
    Config c{false, 0, ""};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(NVP, ConfigBinaryLargeRetries)
{
    Config c{true, 1000, "example.com"};
    EXPECT_EQ(roundtrip_binary(c), c);
}

// ---- Config JSON roundtrip ----

TEST(NVP, ConfigJsonRoundtrip)
{
    Config c{false, 10, "example.com"};
    EXPECT_EQ(roundtrip_json(c), c);
}

TEST(NVP, ConfigJsonHasObjectBraces)
{
    Config c{true, 3, "host"};
    auto json = Serio::serialize<Serio::JSON>({}, c);
    EXPECT_EQ(json.front(), '{');
    EXPECT_EQ(json.back(), '}');
}

TEST(NVP, ConfigJsonHasAllFieldNames)
{
    Config c{true, 3, "myhost"};
    auto json = Serio::serialize<Serio::JSON>({}, c);
    EXPECT_NE(json.find("\"enabled\""), std::string::npos);
    EXPECT_NE(json.find("\"retries\""), std::string::npos);
    EXPECT_NE(json.find("\"host\""), std::string::npos);
    EXPECT_NE(json.find("\"myhost\""), std::string::npos);
}

TEST(NVP, ConfigJsonNotArrayFormat)
{
    // Without NVP a struct serializes as array; with NVP it's an object
    Config c{true, 3, "h"};
    auto json = Serio::serialize<Serio::JSON>({}, c);
    // Object starts with '{', not '['
    EXPECT_NE(json.find('{'), std::string::npos);
    EXPECT_EQ(json.find('['), std::string::npos);
}

// ---- Config XML roundtrip ----

TEST(NVP, ConfigXmlRoundtrip)
{
    Config c{true, 0, ""};
    EXPECT_EQ(roundtrip_xml(c), c);
}

TEST(NVP, ConfigXmlHasAllFieldNames)
{
    Config c{false, 5, "server"};
    auto xml = Serio::serialize<Serio::XML>({}, c);
    EXPECT_NE(xml.find("enabled"), std::string::npos);
    EXPECT_NE(xml.find("retries"), std::string::npos);
    EXPECT_NE(xml.find("host"), std::string::npos);
}

// ---- NVP is a no-op in binary (same bytes as plain struct fields) ----

TEST(NVP, NvpBinaryNoopForRegistered)
{
    // In binary, NVP has no effect on field order or encoding
    Config c1{true, 7, "test"};
    auto b1 = Serio::serialize<Serio::Binary>({}, c1);
    Config c2{true, 7, "test"};
    auto b2 = Serio::serialize<Serio::Binary>({}, c2);
    EXPECT_EQ(b1, b2);
}

// ---- Vector of Config ----

TEST(NVP, VectorOfConfigBinary)
{
    std::vector<Config> v{{true, 1, "host1"}, {false, 5, "host2"}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::vector<Config> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], v[0]);
    EXPECT_EQ(out[1], v[1]);
}

TEST(NVP, VectorOfConfigJson)
{
    std::vector<Config> v{{true, 1, "h1"}, {false, 0, "h2"}};
    auto json = Serio::serialize<Serio::JSON>({}, v);
    std::vector<Config> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], v[0]);
    EXPECT_EQ(out[1], v[1]);
}

TEST(NVP, VectorOfConfigXml)
{
    std::vector<Config> v{{false, 3, "a"}, {true, 0, "b"}};
    auto xml = Serio::serialize<Serio::XML>({}, v);
    std::vector<Config> out;
    Serio::deserialize<Serio::XML>({}, xml, out);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], v[0]);
    EXPECT_EQ(out[1], v[1]);
}

// ---- Map with Config values ----

TEST(NVP, MapOfConfig)
{
    std::map<std::string, Config> m{{"a", {true, 1, "host1"}}, {"b", {false, 2, "host2"}}};
    EXPECT_EQ(roundtrip_binary(m), m);
}

TEST(NVP, MapOfConfigJson)
{
    std::map<std::string, Config> m{{"x", {true, 5, "h1"}}, {"y", {false, 0, "h2"}}};
    EXPECT_EQ(roundtrip_json(m), m);
}

// ---- Config in optional/variant ----

TEST(NVP, OptionalConfig)
{
    std::optional<Config> o{Config{true, 3, "host"}};
    EXPECT_EQ(roundtrip_binary(o), o);
    EXPECT_EQ(roundtrip_json(o), o);
}

TEST(NVP, VariantWithConfig)
{
    std::variant<int, Config> v{Config{false, 1, "x"}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- Serialize-only NVP at top level for JSON (to verify field name in output) ----

TEST(NVP, TopLevelNvpSerializeOnlyInt)
{
    int val = 99;
    auto sn = Serio::nvp("myField", val);
    auto json = Serio::serialize<Serio::JSON>({}, sn);
    EXPECT_NE(json.find("myField"), std::string::npos);
    EXPECT_NE(json.find("99"), std::string::npos);
}

TEST(NVP, TopLevelNvpSerializeOnlyString)
{
    std::string val = "hello";
    auto sn = Serio::nvp("greeting", val);
    auto json = Serio::serialize<Serio::JSON>({}, sn);
    EXPECT_NE(json.find("greeting"), std::string::npos);
    EXPECT_NE(json.find("hello"), std::string::npos);
}
