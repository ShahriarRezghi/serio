#include "common.h"

// ---- std::pair ----

TEST(BinaryPair, IntInt)
{
    std::pair<int, int> p{42, -7};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryPair, StringInt)
{
    std::pair<std::string, int> p{"hello", 99};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryPair, BoolDouble)
{
    std::pair<bool, double> p{true, 3.14};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryPair, VectorVector)
{
    std::pair<std::vector<int>, std::vector<std::string>> p{{1, 2, 3}, {"a", "b"}};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryPair, Nested)
{
    std::pair<std::pair<int, int>, std::pair<std::string, bool>> p{{1, 2}, {"hello", true}};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(BinaryPair, InVector)
{
    std::vector<std::pair<int, std::string>> v{{1, "a"}, {2, "b"}, {3, "c"}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryPair, WithCustomType)
{
    std::pair<Point2D, std::string> p{{1.0f, 2.0f}, "point"};
    EXPECT_EQ(roundtrip_binary(p), p);
}

// ---- std::tuple ----

TEST(BinaryTuple, Empty)
{
    std::tuple<> t{};
    auto bytes = Serio::serialize<Serio::Binary>({}, t);
    std::tuple<> out{};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    // empty tuple — just verify it doesn't throw
}

TEST(BinaryTuple, Single)
{
    std::tuple<int> t{42};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, TwoElements)
{
    std::tuple<int, std::string> t{1, "hello"};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, ThreeElements)
{
    std::tuple<bool, int, double> t{true, 42, 3.14};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, FourElements)
{
    std::tuple<int, float, std::string, bool> t{1, 2.5f, "hi", false};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, ManyElements)
{
    auto t = std::make_tuple(uint8_t(1), int16_t(2), int32_t(3), int64_t(4), float(5.0f), double(6.0),
                             std::string("seven"), bool(true));
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, WithContainers)
{
    std::tuple<std::vector<int>, std::set<std::string>> t{{1, 2, 3}, {"a", "b"}};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, Nested)
{
    std::tuple<std::tuple<int, int>, double> t{{1, 2}, 3.14};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, WithCustomType)
{
    std::tuple<Point2D, Point3D> t{{1.0f, 2.0f}, {3.0, 4.0, 5.0}};
    EXPECT_EQ(roundtrip_binary(t), t);
}

TEST(BinaryTuple, VectorOfTuples)
{
    std::vector<std::tuple<int, std::string, bool>> v{{1, "a", true}, {2, "b", false}, {3, "c", true}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryTuple, MapToTuple)
{
    std::map<int, std::tuple<std::string, double>> m{{1, {"one", 1.0}}, {2, {"two", 2.0}}};
    EXPECT_EQ(roundtrip_binary(m), m);
}

// ---- Pair / tuple interop with map ----

TEST(BinaryPair, FromMap)
{
    // A map can be deserialized as a vector of pairs
    std::map<int, std::string> m{{1, "a"}, {2, "b"}, {3, "c"}};
    auto bytes = Serio::serialize<Serio::Binary>({}, m);
    std::vector<std::pair<int, std::string>> v;
    Serio::deserialize<Serio::Binary>({}, bytes, v);
    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0].first, 1);
    EXPECT_EQ(v[0].second, "a");
    EXPECT_EQ(v[1].first, 2);
    EXPECT_EQ(v[1].second, "b");
    EXPECT_EQ(v[2].first, 3);
    EXPECT_EQ(v[2].second, "c");
}

TEST(BinaryPair, ToMap)
{
    std::vector<std::pair<int, std::string>> v{{1, "a"}, {2, "b"}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::map<int, std::string> m;
    Serio::deserialize<Serio::Binary>({}, bytes, m);
    EXPECT_EQ(m.size(), 2u);
    EXPECT_EQ(m[1], "a");
    EXPECT_EQ(m[2], "b");
}
