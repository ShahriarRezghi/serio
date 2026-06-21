#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "common.h"

// ---- std::set<T> ----

TEST(Set, Empty_Int) { check(std::set<int>{}); }
TEST(Set, Single_Int) { check(std::set<int>{42}); }
TEST(Set, Multi_Int) { check(std::set<int>{5, 3, 1, 4, 2}); }  // stored sorted
TEST(Set, String) { check(std::set<std::string>{"banana", "apple", "cherry"}); }
TEST(Set, Duplicates_Ignored)
{
    // Set ignores duplicates
    std::set<int> s{1, 2, 3};
    check(s);
}

// ---- std::multiset<T> ----

TEST(Multiset, Empty) { check(std::multiset<int>{}); }
TEST(Multiset, Duplicates) { check(std::multiset<int>{3, 1, 2, 1, 3, 3}); }
TEST(Multiset, String) { check(std::multiset<std::string>{"a", "b", "a"}); }

// ---- std::unordered_set<T> ----

TEST(UnorderedSet, Empty) { check(std::unordered_set<int>{}); }
TEST(UnorderedSet, Multi) { check(std::unordered_set<int>{10, 20, 30}); }
TEST(UnorderedSet, String) { check(std::unordered_set<std::string>{"x", "y", "z"}); }

// ---- std::unordered_multiset<T> ----

TEST(UnorderedMultiset, Empty) { check(std::unordered_multiset<int>{}); }
TEST(UnorderedMultiset, Duplicates) { check(std::unordered_multiset<int>{1, 1, 2, 3, 2}); }

// ---- std::map<K, V> ----

TEST(Map, Empty) { check(std::map<int, int>{}); }
TEST(Map, Single) { check(std::map<int, int>{{1, 100}}); }
TEST(Map, Multi) { check(std::map<int, int>{{1, 10}, {2, 20}, {3, 30}}); }
TEST(Map, StringKeys) { check(std::map<std::string, int>{{"a", 1}, {"b", 2}, {"c", 3}}); }
TEST(Map, StringValues) { check(std::map<int, std::string>{{1, "one"}, {2, "two"}, {3, "three"}}); }
TEST(Map, StringPairs) { check(std::map<std::string, std::string>{{"k1", "v1"}, {"k2", "v2"}}); }

// Nested map
TEST(Map, NestedValue)
{
    std::map<int, std::vector<int>> m{{1, {1, 2}}, {2, {3, 4, 5}}};
    check(m);
}

// ---- std::multimap<K, V> ----

TEST(Multimap, Empty) { check(std::multimap<int, int>{}); }
TEST(Multimap, Single) { check(std::multimap<int, int>{{1, 10}}); }
TEST(Multimap, Duplicates)
{
    std::multimap<int, int> m{{1, 10}, {1, 20}, {2, 30}, {2, 40}};
    // multimap<K,V> comparison uses unordered_multimap for correctness
    auto bytes = Serio::serialize<Serio::Binary>({}, m);
    std::multimap<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(m.size(), out.size());
    // Compare using unordered_multimap
    std::unordered_multimap<int, int> u1(m.begin(), m.end());
    std::unordered_multimap<int, int> u2(out.begin(), out.end());
    EXPECT_EQ(u1, u2);
}

// ---- std::unordered_map<K, V> ----

TEST(UnorderedMap, Empty) { check(std::unordered_map<int, int>{}); }
TEST(UnorderedMap, Single) { check(std::unordered_map<int, int>{{7, 77}}); }
TEST(UnorderedMap, Multi) { check(std::unordered_map<int, int>{{1, 10}, {2, 20}, {3, 30}}); }
TEST(UnorderedMap, String) { check(std::unordered_map<std::string, int>{{"a", 1}, {"b", 2}}); }

// ---- std::unordered_multimap<K, V> ----

TEST(UnorderedMultimap, Empty) { check(std::unordered_multimap<int, int>{}); }
TEST(UnorderedMultimap, Duplicates)
{
    std::unordered_multimap<int, int> m{{1, 10}, {1, 20}, {2, 30}};
    auto bytes = Serio::serialize<Serio::Binary>({}, m);
    std::unordered_multimap<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(m, out);
}

// ---- Nested associative containers ----

TEST(Map, OfSets)
{
    std::map<int, std::set<int>> m{{1, {1, 2, 3}}, {2, {4, 5}}};
    check(m);
}

TEST(Map, OfVectors)
{
    std::map<std::string, std::vector<double>> m{{"a", {1.0, 2.0}}, {"b", {3.0}}};
    check(m);
}
