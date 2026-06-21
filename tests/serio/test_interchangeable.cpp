#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common.h"

// Resizable sequences share the layout [count][elements...] and can be
// freely swapped between serialization and deserialization.

// ---- vector <-> list ----

TEST(Interchangeable, VectorToList)
{
    std::vector<int> orig{3, 1, 4, 1, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::list<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(std::vector<int>(out.begin(), out.end()), orig);
}

TEST(Interchangeable, ListToVector)
{
    std::list<int> orig{1, 2, 3, 4};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, std::vector<int>(orig.begin(), orig.end()));
}

// ---- vector <-> deque ----

TEST(Interchangeable, VectorToDeque)
{
    std::vector<int> orig{10, 20, 30};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::deque<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(std::vector<int>(out.begin(), out.end()), orig);
}

TEST(Interchangeable, DequeToVector)
{
    std::deque<double> orig{1.1, 2.2, 3.3};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, std::vector<double>(orig.begin(), orig.end()));
}

// ---- vector <-> forward_list ----

TEST(Interchangeable, VectorToForwardList)
{
    std::vector<int> orig{5, 6, 7, 8};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::forward_list<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    std::vector<int> result(out.begin(), out.end());
    EXPECT_EQ(result, orig);
}

// ---- vector <-> set ----

TEST(Interchangeable, VectorToSet)
{
    // Duplicates are removed and order becomes sorted
    std::vector<int> orig{3, 1, 4, 1, 5, 9, 2, 6};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::set<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, std::set<int>(orig.begin(), orig.end()));
}

TEST(Interchangeable, SetToVector)
{
    std::set<int> orig{1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, std::vector<int>(orig.begin(), orig.end()));
}

// ---- vector <-> multiset ----

TEST(Interchangeable, VectorToMultiset)
{
    std::vector<int> orig{3, 1, 4, 1, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::multiset<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    std::vector<int> ms_out(out.begin(), out.end());
    EXPECT_EQ(ms_out, (std::vector<int>{1, 1, 3, 4, 5}));
}

TEST(Interchangeable, MultisetToVector)
{
    std::multiset<int> orig{3, 1, 2, 1, 3};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, (std::vector<int>{1, 1, 2, 3, 3}));
}

// ---- vector <-> unordered_set ----

TEST(Interchangeable, VectorToUnorderedSet)
{
    std::vector<int> orig{3, 1, 4, 1, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::unordered_set<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    std::set<int> expected(orig.begin(), orig.end());
    EXPECT_EQ(out, (std::unordered_set<int>(expected.begin(), expected.end())));
}

// ---- vector<bool> stays vector<bool> ----

TEST(Interchangeable, VectorBoolRoundtrip)
{
    std::vector<bool> v{true, false, true, true, false};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::vector<bool> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(v, out);
}

// ---- vector<string> <-> list<string> ----

TEST(Interchangeable, VectorStringToList)
{
    auto bytes = Serio::serialize<Serio::Binary>({}, std::vector<std::string>{"a", "b", "c"});
    std::list<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(std::list<std::string>({"a", "b", "c"}), out);
}

// ---- valarray <-> vector ----

TEST(Interchangeable, ValArrayToVector)
{
    std::valarray<int> orig{1, 2, 3, 4};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, (std::vector<int>{1, 2, 3, 4}));
}

TEST(Interchangeable, VectorToValArray)
{
    std::vector<int> orig{5, 6, 7};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::valarray<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), orig.size());
    for (std::size_t i = 0; i < orig.size(); ++i) EXPECT_EQ(out[i], orig[i]);
}

// ---- map <-> vector<pair> ----

TEST(Interchangeable, MapToVectorOfPairs)
{
    std::map<int, int> orig{{1, 10}, {2, 20}, {3, 30}};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<std::pair<int, int>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], std::make_pair(1, 10));
    EXPECT_EQ(out[1], std::make_pair(2, 20));
    EXPECT_EQ(out[2], std::make_pair(3, 30));
}

TEST(Interchangeable, VectorOfPairsToMap)
{
    std::vector<std::pair<int, std::string>> orig{{1, "a"}, {2, "b"}, {3, "c"}};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::map<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), 3u);
    EXPECT_EQ(out[1], "a");
    EXPECT_EQ(out[2], "b");
    EXPECT_EQ(out[3], "c");
}

TEST(Interchangeable, VectorOfPairsToUnorderedMap)
{
    std::vector<std::pair<int, int>> orig{{1, 10}, {2, 20}};
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::unordered_map<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), 2u);
    EXPECT_EQ(out[1], 10);
    EXPECT_EQ(out[2], 20);
}

TEST(Interchangeable, MapToUnorderedMap)
{
    auto bytes = Serio::serialize<Serio::Binary>({}, std::map<int, std::string>{{1, "a"}, {2, "b"}});
    std::unordered_map<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ((std::unordered_map<int, std::string>{{1, "a"}, {2, "b"}}), out);
}

// ---- Fixed-size containers are NOT interchangeable with resizable ones ----

TEST(Interchangeable, ArrayNotInterchangeableWithVector)
{
    // Serialized sizes differ: array has no count prefix
    std::array<int, 3> arr{1, 2, 3};
    std::vector<int> vec{1, 2, 3};
    auto arr_b = Serio::serialize<Serio::Binary>({}, arr);
    auto vec_b = Serio::serialize<Serio::Binary>({}, vec);
    EXPECT_NE(arr_b, vec_b);
}

// ---- StaticArrayView <-> std::array ----

TEST(Interchangeable, StaticArrayViewToArray)
{
    int buf[3] = {7, 8, 9};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<int, 3>(buf));
    std::array<int, 3> out{};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out[0], 7);
    EXPECT_EQ(out[1], 8);
    EXPECT_EQ(out[2], 9);
}

TEST(Interchangeable, ArrayToStaticArrayView)
{
    std::array<int, 3> arr{10, 11, 12};
    auto bytes = Serio::serialize<Serio::Binary>({}, arr);
    int out[3] = {};
    auto v = Serio::StaticArrayView<int, 3>(out);
    Serio::deserialize<Serio::Binary>({}, bytes, v);
    EXPECT_EQ(out[0], 10);
    EXPECT_EQ(out[1], 11);
    EXPECT_EQ(out[2], 12);
}

// ---- PointerView <-> resizable containers ----

TEST(Interchangeable, PointerViewToList)
{
    int arr[] = {1, 2, 3, 4};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<int>(arr, 4));
    std::list<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(std::vector<int>(out.begin(), out.end()), (std::vector<int>{1, 2, 3, 4}));
}

TEST(Interchangeable, PointerViewToDeque)
{
    int arr[] = {9, 8, 7};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<int>(arr, 3));
    std::deque<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(std::vector<int>(out.begin(), out.end()), (std::vector<int>{9, 8, 7}));
}

// ---- Empty containers round-trip through different types ----

TEST(Interchangeable, EmptyVectorToEmptyList)
{
    std::vector<int> orig;
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::list<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}

TEST(Interchangeable, EmptySetToEmptyVector)
{
    std::set<std::string> orig;
    auto bytes = Serio::serialize<Serio::Binary>({}, orig);
    std::vector<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}
