#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <span>
#include <string>
#include <valarray>
#include <vector>

#include "common.h"

// ---- std::vector<T> ----

TEST(Vector, Empty_Int) { check(std::vector<int>{}); }
TEST(Vector, Single_Int) { check(std::vector<int>{42}); }
TEST(Vector, Multi_Int) { check(std::vector<int>{1, 2, 3, 4, 5}); }
TEST(Vector, Large_Int) { check(std::vector<int>(200, 7)); }
TEST(Vector, Negative_Int) { check(std::vector<int>{-100, -1, 0, 1, 100}); }

TEST(Vector, Empty_Double) { check(std::vector<double>{}); }
TEST(Vector, Multi_Double) { check(std::vector<double>{1.0, 0.5, 0.25, -1.0}); }

TEST(Vector, Empty_String) { check(std::vector<std::string>{}); }
TEST(Vector, Multi_String) { check(std::vector<std::string>{"a", "bb", "ccc"}); }

// Nested vector
TEST(Vector, Nested)
{
    std::vector<std::vector<int>> v{{1, 2}, {3, 4, 5}, {}, {6}};
    check(v);
}

TEST(Vector, TriplyNested)
{
    std::vector<std::vector<std::vector<int>>> v{{{1, 2}, {3}}, {{4, 5, 6}}};
    check(v);
}

// ---- std::vector<bool> (bit-packed specialization) ----

TEST(VectorBool, Empty) { check(std::vector<bool>{}); }
TEST(VectorBool, AllFalse) { check(std::vector<bool>(10, false)); }
TEST(VectorBool, AllTrue) { check(std::vector<bool>(10, true)); }
TEST(VectorBool, Mixed) { check(std::vector<bool>{true, false, true, true, false}); }
TEST(VectorBool, Single) { check(std::vector<bool>{true}); }
TEST(VectorBool, Large)
{
    std::vector<bool> v(257);
    for (size_t i = 0; i < v.size(); ++i) v[i] = (i % 3 == 0);
    check(v);
}

// vector<bool> only round-trips as vector<bool> (bit-packed), not as list<bool>
TEST(VectorBool, NotInterchangeableWithList)
{
    // Serialize as vector<bool>, must deserialize as vector<bool> — not list<bool>
    std::vector<bool> v{true, false, true};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::vector<bool> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(v, out);
}

// ---- std::list<T> ----

TEST(List, Empty_Int) { check(std::list<int>{}); }
TEST(List, Single_Int) { check(std::list<int>{42}); }
TEST(List, Multi_Int) { check(std::list<int>{1, 2, 3, 4, 5}); }

TEST(List, String) { check(std::list<std::string>{"hello", "world"}); }
TEST(List, Nested)
{
    std::list<std::list<int>> v{{1, 2}, {3, 4}};
    check(v);
}

// ---- std::deque<T> ----

TEST(Deque, Empty) { check(std::deque<int>{}); }
TEST(Deque, Single) { check(std::deque<int>{100}); }
TEST(Deque, Multi) { check(std::deque<int>{10, 20, 30, 40}); }
TEST(Deque, String) { check(std::deque<std::string>{"a", "b", "c"}); }

// ---- std::forward_list<T> ----

TEST(ForwardList, Empty) { check(std::forward_list<int>{}); }
TEST(ForwardList, Single) { check(std::forward_list<int>{7}); }
TEST(ForwardList, Multi) { check(std::forward_list<int>{1, 2, 3, 4, 5}); }
TEST(ForwardList, String) { check(std::forward_list<std::string>{"x", "y"}); }

// ---- std::valarray<T> ----

TEST(Valarray, Empty_Int) { check_valarray(std::valarray<int>{}); }
TEST(Valarray, Multi_Int) { check_valarray(std::valarray<int>{1, 2, 3, 4, 5}); }
TEST(Valarray, Multi_Double) { check_valarray(std::valarray<double>{1.0, 0.5, -1.0}); }
TEST(Valarray, Large)
{
    std::valarray<int> v(100);
    for (int i = 0; i < 100; ++i) v[i] = i * i;
    check_valarray(v);
}

// ---- std::array<T, N> (fixed size, no length prefix) ----

TEST(Array, Zero_N1) { check(std::array<int, 1>{0}); }
TEST(Array, Multi_N5) { check(std::array<int, 5>{10, 20, 30, 40, 50}); }
TEST(Array, Multi_N10) { check(std::array<double, 10>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0}); }
TEST(Array, String_N3) { check(std::array<std::string, 3>{"a", "bb", "ccc"}); }
TEST(Array, Bool_N8) { check(std::array<bool, 8>{true, false, true, false, true, false, true, false}); }

// std::array writes no length prefix — can only round-trip to same N
TEST(Array, NoLengthPrefix)
{
    // array<int,3> written has no length prefix; reading into array<int,3> is fine
    std::array<int, 3> a{7, 8, 9};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::array<int, 3> b{};
    Serio::deserialize<Serio::Binary>({}, bytes, b);
    EXPECT_EQ(a, b);
}

// Nested array
TEST(Array, Nested)
{
    std::array<std::array<int, 2>, 3>{{{1, 2}, {3, 4}, {5, 6}}};
    check(std::array<std::array<int, 2>, 3>{{{1, 2}, {3, 4}, {5, 6}}});
}

// ---- std::span<T, N> (C++20, serialize-only) ----

TEST(Span, SerializeOnlyFixed)
{
    // Fixed-extent span serializes like array (no length prefix)
    int buf[5] = {10, 20, 30, 40, 50};
    std::span<int, 5> sp(buf);
    auto bytes = Serio::serialize<Serio::Binary>({}, sp);

    // Deserialize into array of same size
    std::array<int, 5> arr{};
    Serio::deserialize<Serio::Binary>({}, bytes, arr);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(buf[i], arr[i]);
}

TEST(Span, SerializeOnlyFixed_JSON)
{
    int buf[5] = {10, 20, 30, 40, 50};
    std::span<int, 5> sp(buf);
    auto j = Serio::serialize<Serio::JSON>({}, sp);
    std::array<int, 5> arr{};
    Serio::deserialize<Serio::JSON>({}, j, arr);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(buf[i], arr[i]);
}

TEST(Span, SerializeOnlyFixed_XML)
{
    int buf[5] = {10, 20, 30, 40, 50};
    std::span<int, 5> sp(buf);
    auto x = Serio::serialize<Serio::XML>({}, sp);
    std::array<int, 5> arr{};
    Serio::deserialize<Serio::XML>({}, x, arr);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(buf[i], arr[i]);
}

// ---- std::span<T> (dynamic_extent, C++20, serialize-only) ----

TEST(Span, SerializeOnlyDynamic_Binary)
{
    std::vector<int> src{1, 2, 3, 4, 5};
    std::span<int> sp(src);
    auto bytes = Serio::serialize<Serio::Binary>({}, sp);
    std::vector<int> dst;
    Serio::deserialize<Serio::Binary>({}, bytes, dst);
    EXPECT_EQ(src, dst);
}

TEST(Span, SerializeOnlyDynamic_JSON)
{
    std::vector<int> src{1, 2, 3, 4, 5};
    std::span<int> sp(src);
    auto j = Serio::serialize<Serio::JSON>({}, sp);
    std::vector<int> dst;
    Serio::deserialize<Serio::JSON>({}, j, dst);
    EXPECT_EQ(src, dst);
}

TEST(Span, SerializeOnlyDynamic_XML)
{
    std::vector<int> src{1, 2, 3, 4, 5};
    std::span<int> sp(src);
    auto x = Serio::serialize<Serio::XML>({}, sp);
    std::vector<int> dst;
    Serio::deserialize<Serio::XML>({}, x, dst);
    EXPECT_EQ(src, dst);
}

TEST(Span, SerializeOnlyDynamic_Empty)
{
    std::span<int> sp;
    auto bytes = Serio::serialize<Serio::Binary>({}, sp);
    std::vector<int> dst{1, 2, 3};
    Serio::deserialize<Serio::Binary>({}, bytes, dst);
    EXPECT_TRUE(dst.empty());
}

TEST(Span, SerializeOnlyDynamic_Strings)
{
    std::vector<std::string> src{"hello", "world", "serio"};
    std::span<std::string> sp(src);
    auto bytes = Serio::serialize<Serio::Binary>({}, sp);
    std::vector<std::string> dst;
    Serio::deserialize<Serio::Binary>({}, bytes, dst);
    EXPECT_EQ(src, dst);
}
