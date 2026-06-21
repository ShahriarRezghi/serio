#include "common.h"

// Helper: forward_list equality (no operator== for forward_list until C++20 but we have 23)
template <typename T>
bool fl_eq(const std::forward_list<T>& a, const std::forward_list<T>& b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

// ---- std::list ----

TEST(BinaryList, Empty)
{
    std::list<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryList, Simple)
{
    std::list<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryList, Strings)
{
    std::list<std::string> v{"a", "bb", "ccc"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryList, Nested)
{
    std::list<std::list<int>> v{{1, 2}, {3, 4, 5}, {}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryList, Large)
{
    std::list<double> v;
    for (int i = 0; i < 5000; ++i) v.push_back(i * 0.1);
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::deque ----

TEST(BinaryDeque, Empty)
{
    std::deque<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryDeque, Simple)
{
    std::deque<int> v{10, 20, 30, 40};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryDeque, Strings)
{
    std::deque<std::string> v{"x", "y", "z"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryDeque, Large)
{
    std::deque<int64_t> v(2000, -1LL);
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::forward_list ----

TEST(BinaryForwardList, Empty)
{
    std::forward_list<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::forward_list<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(fl_eq(v, out));
}

TEST(BinaryForwardList, Simple)
{
    std::forward_list<int> v{5, 4, 3, 2, 1};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::forward_list<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(fl_eq(v, out));
}

TEST(BinaryForwardList, Strings)
{
    std::forward_list<std::string> v{"alpha", "beta", "gamma"};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::forward_list<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(fl_eq(v, out));
}

// ---- std::valarray ----

TEST(BinaryValarray, Empty)
{
    std::valarray<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::valarray<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), 0u);
}

TEST(BinaryValarray, Simple)
{
    std::valarray<int> v{1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::valarray<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), v.size());
    for (std::size_t i = 0; i < v.size(); ++i) EXPECT_EQ(out[i], v[i]);
}

TEST(BinaryValarray, Doubles)
{
    std::valarray<double> v{1.1, 2.2, 3.3};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::valarray<double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), v.size());
    for (std::size_t i = 0; i < v.size(); ++i) EXPECT_EQ(out[i], v[i]);
}

TEST(BinaryValarray, Large)
{
    std::valarray<float> v(3000);
    for (std::size_t i = 0; i < v.size(); ++i) v[i] = static_cast<float>(i) * 0.5f;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::valarray<float> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), v.size());
    for (std::size_t i = 0; i < v.size(); ++i) EXPECT_EQ(out[i], v[i]);
}

// ---- std::array (fixed-size, no length prefix) ----

TEST(BinaryArray, Int5)
{
    std::array<int, 5> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryArray, Size1)
{
    std::array<int, 1> v{99};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryArray, Doubles)
{
    std::array<double, 4> v{1.1, 2.2, 3.3, 4.4};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryArray, Strings)
{
    std::array<std::string, 3> v{"a", "bb", "ccc"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryArray, Zero)
{
    std::array<int, 4> v{0, 0, 0, 0};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryArray, CustomType)
{
    std::array<Point2D, 2> v{Point2D{1.0f, 2.0f}, Point2D{-3.0f, 4.5f}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryArray, Large)
{
    std::array<int, 100> v;
    for (int i = 0; i < 100; ++i) v[i] = i * 2;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryArray, NoLengthPrefix)
{
    // Verify std::array and std::vector produce different byte streams
    // (array has no count prefix)
    std::array<int, 3> arr{1, 2, 3};
    std::vector<int> vec{1, 2, 3};
    auto arr_bytes = Serio::serialize<Serio::Binary>({}, arr);
    auto vec_bytes = Serio::serialize<Serio::Binary>({}, vec);
    EXPECT_NE(arr_bytes, vec_bytes);
}

// ---- std::span (serialize-only, C++20) ----

TEST(BinarySpan, FixedExtent)
{
    int arr[5] = {10, 20, 30, 40, 50};
    std::span<int, 5> sp(arr);
    auto bytes = Serio::serialize<Serio::Binary>({}, sp);
    std::array<int, 5> out{};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(out[i], arr[i]);
}

TEST(BinarySpan, DynamicExtent)
{
    std::vector<int> v{1, 2, 3, 4};
    std::span<int> sp(v);
    auto bytes = Serio::serialize<Serio::Binary>({}, sp);
    // dynamic span produces a length-prefixed sequence
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(BinarySpan, DynamicExtentEmpty)
{
    std::vector<int> v;
    std::span<int> sp(v);
    auto bytes = Serio::serialize<Serio::Binary>({}, sp);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}
