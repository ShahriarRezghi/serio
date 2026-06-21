#include "common.h"

// ---- Deeply nested types ----

TEST(Nested, VectorVectorVector)
{
    std::vector<std::vector<std::vector<int>>> v{{{1, 2}, {3}}, {{4, 5, 6}}, {{}}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(Nested, MapOfVectorOfSet)
{
    std::map<std::string, std::vector<std::set<int>>> m{{"a", {{1, 2}, {3}}}, {"b", {{4, 5, 6}}}};
    EXPECT_EQ(roundtrip_binary(m), m);
}

TEST(Nested, VectorOfPairOfVectors)
{
    std::vector<std::pair<std::vector<int>, std::vector<std::string>>> v{
        {{1, 2, 3}, {"a", "b"}}, {{}, {}}, {{4}, {"c", "d", "e"}}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(Nested, TupleOfContainers)
{
    std::tuple<std::vector<int>, std::set<std::string>, std::map<int, double>> t{
        {1, 2, 3}, {"x", "y"}, {{1, 1.1}, {2, 2.2}}};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(Nested, OptionalOfOptionalOfVector)
{
    std::optional<std::optional<std::vector<int>>> o{std::optional<std::vector<int>>{std::vector<int>{1, 2, 3}}};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(Nested, VariantOfVectorOfOptional)
{
    using V = std::variant<std::vector<std::optional<int>>, std::string>;
    V v{std::vector<std::optional<int>>{1, std::nullopt, 3}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(Nested, SharedPtrOfVector)
{
    auto p = std::make_shared<std::vector<std::map<int, std::string>>>(
        std::vector<std::map<int, std::string>>{{{1, "a"}}, {{2, "b"}, {3, "c"}}});
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<std::vector<std::map<int, std::string>>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, *p);
}

TEST(Nested, MapOfOptionals)
{
    std::map<int, std::optional<std::string>> m{{1, "hello"}, {2, std::nullopt}, {3, "world"}};
    EXPECT_EQ(roundtrip_binary(m), m);
}

TEST(Nested, VectorOfTuples)
{
    std::vector<std::tuple<int, std::string, std::vector<double>>> v{
        {1, "a", {1.1, 2.2}}, {2, "b", {}}, {3, "c", {3.3}}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(Nested, DeepMapNesting)
{
    std::map<int, std::map<int, std::map<int, std::string>>> m{{1, {{2, {{3, "deep"}}}}}};
    EXPECT_EQ(roundtrip_binary(m), m);
}

TEST(Nested, CustomTypeInAllFormats)
{
    Nested n{Point2D{1.0f, 2.0f}, {1, 2, 3}, "label"};
    EXPECT_EQ(roundtrip_binary(n), n);
    EXPECT_EQ(roundtrip_json(n), n);
    EXPECT_EQ(roundtrip_xml(n), n);
}

TEST(Nested, ComplexVectors)
{
    std::vector<std::complex<double>> v{{1.0, 2.0}, {-3.0, 4.0}, {0.0, 0.0}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(Nested, VectorOfBitsets)
{
    std::vector<std::bitset<8>> v{std::bitset<8>(0b10101010), std::bitset<8>(0b01010101), std::bitset<8>(0b11001100)};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::vector<std::bitset<8>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 3u);
    for (std::size_t i = 0; i < v.size(); ++i) EXPECT_EQ(out[i], v[i]);
}

TEST(Nested, ChronoInVector)
{
    std::vector<std::chrono::milliseconds> v{std::chrono::milliseconds(100), std::chrono::milliseconds(200),
                                             std::chrono::milliseconds(0)};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(Nested, MapOfCustomInJson)
{
    std::map<std::string, Point2D> m{{"a", {1.0f, 2.0f}}, {"b", {3.0f, 4.0f}}};
    EXPECT_EQ(roundtrip_json(m), m);
}

TEST(Nested, OptionalCustomInJson)
{
    std::optional<Point2D> o{Point2D{1.5f, 2.5f}};
    EXPECT_EQ(roundtrip_json(o), o);
}

TEST(Nested, LargeNestedStructure)
{
    std::map<std::string, std::vector<std::pair<int, std::string>>> m;
    for (int i = 0; i < 10; ++i)
    {
        std::string key = "key" + std::to_string(i);
        std::vector<std::pair<int, std::string>> v;
        for (int j = 0; j < 20; ++j) v.push_back({j, "val_" + std::to_string(j)});
        m[key] = v;
    }
    EXPECT_EQ(roundtrip_binary(m), m);
}
