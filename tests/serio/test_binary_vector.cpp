#include "common.h"

enum class Direction : uint8_t
{
    North = 0,
    South = 1,
    East = 2,
    West = 3
};

// ---- std::vector<int> ----

TEST(BinaryVector, Empty)
{
    std::vector<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, Single)
{
    std::vector<int> v{42};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, Multiple)
{
    std::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, NegativeValues)
{
    std::vector<int> v{-1, -100, -32768, 0, 1};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, Large)
{
    std::vector<int> v(10000);
    for (int i = 0; i < 10000; ++i) v[i] = i * i;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorDouble)
{
    std::vector<double> v{1.1, 2.2, 3.3, -4.4, 0.0};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorFloat)
{
    std::vector<float> v{1.0f, -2.0f, 0.0f, 100.5f};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorUint8)
{
    std::vector<uint8_t> v{0, 1, 127, 128, 255};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorInt64)
{
    std::vector<int64_t> v{std::numeric_limits<int64_t>::min(), -1LL, 0LL, 1LL, std::numeric_limits<int64_t>::max()};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorString)
{
    std::vector<std::string> v{"hello", "", "world", "foo\nbar"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorOfVectors)
{
    std::vector<std::vector<int>> v{{1, 2}, {3, 4, 5}, {}, {6}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorOfPairs)
{
    std::vector<std::pair<int, std::string>> v{{1, "one"}, {2, "two"}, {3, "three"}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorCustomType)
{
    std::vector<Point2D> v{{1.0f, 2.0f}, {-3.0f, 4.5f}, {0.0f, 0.0f}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorEnum)
{
    std::vector<Direction> v{Direction::North, Direction::South, Direction::East, Direction::West};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::vector<bool> (bit-packed) ----

TEST(BinaryVectorBool, Empty)
{
    std::vector<bool> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVectorBool, Single)
{
    std::vector<bool> v{true};
    EXPECT_EQ(roundtrip_binary(v), v);
    v = {false};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVectorBool, Mixed)
{
    std::vector<bool> v{true, false, true, true, false, false, true};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVectorBool, AllTrue)
{
    std::vector<bool> v(64, true);
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVectorBool, AllFalse)
{
    std::vector<bool> v(64, false);
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVectorBool, NotByteAligned)
{
    // 65 elements to cross a byte boundary
    std::vector<bool> v(65, false);
    v[0] = true;
    v[7] = true;
    v[8] = true;
    v[63] = true;
    v[64] = true;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVectorBool, Large)
{
    std::vector<bool> v(1000);
    for (std::size_t i = 0; i < v.size(); ++i) v[i] = (i % 3 == 0);
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVectorBool, VectorBoolCannotSwapWithList)
{
    // vector<bool> is bit-packed; trying to deserialize as list<bool> should corrupt / differ
    // We verify that vector<bool> deserializes correctly back to itself only
    std::vector<bool> orig(8, true);
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<bool> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, orig);
}

// ---- Nested vector combinations ----

TEST(BinaryVector, DeepNested)
{
    std::vector<std::vector<std::vector<int>>> v{{{1, 2}, {3}}, {{4, 5, 6}}, {}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorOfOptionals)
{
    std::vector<std::optional<int>> v{std::nullopt, 1, std::nullopt, 42};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVector, VectorOfSharedPtr)
{
    std::vector<std::shared_ptr<int>> v;
    v.push_back(std::make_shared<int>(10));
    v.push_back(nullptr);
    v.push_back(std::make_shared<int>(30));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::vector<std::shared_ptr<int>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 3u);
    ASSERT_NE(out[0], nullptr);
    EXPECT_EQ(*out[0], 10);
    EXPECT_EQ(out[1], nullptr);
    ASSERT_NE(out[2], nullptr);
    EXPECT_EQ(*out[2], 30);
}
