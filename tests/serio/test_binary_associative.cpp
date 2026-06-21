#include "common.h"

// ---- std::set ----

TEST(BinarySet, Empty)
{
    std::set<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinarySet, Ints)
{
    std::set<int> v{3, 1, 4, 1, 5, 9, 2, 6};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinarySet, Strings)
{
    std::set<std::string> v{"banana", "apple", "cherry"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinarySet, Large)
{
    std::set<int> v;
    for (int i = 0; i < 1000; ++i) v.insert(i * 7);
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::multiset ----

TEST(BinaryMultiset, Empty)
{
    std::multiset<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMultiset, WithDuplicates)
{
    std::multiset<int> v{1, 1, 2, 2, 3};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMultiset, AllSame)
{
    std::multiset<int> v{5, 5, 5, 5, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::unordered_set ----

TEST(BinaryUnorderedSet, Empty)
{
    std::unordered_set<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryUnorderedSet, Ints)
{
    std::unordered_set<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryUnorderedSet, Strings)
{
    std::unordered_set<std::string> v{"x", "y", "z"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::unordered_multiset ----

TEST(BinaryUnorderedMultiset, Empty)
{
    std::unordered_multiset<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryUnorderedMultiset, WithDuplicates)
{
    std::unordered_multiset<int> v{1, 1, 2, 3, 3};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::unordered_multiset<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- std::map ----

TEST(BinaryMap, Empty)
{
    std::map<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMap, IntToInt)
{
    std::map<int, int> v{{1, 10}, {2, 20}, {3, 30}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMap, StringToInt)
{
    std::map<std::string, int> v{{"a", 1}, {"b", 2}, {"c", 3}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMap, IntToString)
{
    std::map<int, std::string> v{{1, "one"}, {2, "two"}, {-1, "neg"}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMap, StringToVector)
{
    std::map<std::string, std::vector<int>> v{{"a", {1, 2}}, {"b", {3, 4, 5}}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMap, Large)
{
    std::map<int, double> v;
    for (int i = 0; i < 500; ++i) v[i] = i * 3.14;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMap, NestedMaps)
{
    std::map<int, std::map<int, int>> v{{1, {{2, 3}}}, {4, {{5, 6}, {7, 8}}}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::multimap ----

TEST(BinaryMultimap, Empty)
{
    std::multimap<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryMultimap, WithDuplicateKeys)
{
    // std::multimap doesn't guarantee order among equal keys after roundtrip;
    // verify element counts and that keys/values are present
    std::multimap<int, std::string> v{{1, "a"}, {1, "b"}, {2, "c"}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::multimap<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), v.size());
    EXPECT_EQ(out.count(1), 2u);
    EXPECT_EQ(out.count(2), 1u);
    auto r2 = out.equal_range(2);
    EXPECT_EQ(r2.first->second, "c");
}

// ---- std::unordered_map ----

TEST(BinaryUnorderedMap, Empty)
{
    std::unordered_map<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryUnorderedMap, IntToInt)
{
    std::unordered_map<int, int> v{{1, 100}, {2, 200}, {3, 300}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryUnorderedMap, StringToString)
{
    std::unordered_map<std::string, std::string> v{{"hello", "world"}, {"foo", "bar"}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryUnorderedMap, Large)
{
    std::unordered_map<int, int> v;
    for (int i = 0; i < 1000; ++i) v[i] = i * i;
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::unordered_multimap ----

TEST(BinaryUnorderedMultimap, Empty)
{
    std::unordered_multimap<int, int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::unordered_multimap<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(BinaryUnorderedMultimap, WithDuplicateKeys)
{
    std::unordered_multimap<std::string, int> v{{"a", 1}, {"a", 2}, {"b", 3}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::unordered_multimap<std::string, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- Mixed associative containers ----

TEST(BinaryAssociative, SetInMap)
{
    std::map<int, std::set<std::string>> v{{1, {"a", "b"}}, {2, {"c"}}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryAssociative, MapInVector)
{
    std::vector<std::map<int, int>> v{{{1, 2}, {3, 4}}, {}};
    EXPECT_EQ(roundtrip_binary(v), v);
}
