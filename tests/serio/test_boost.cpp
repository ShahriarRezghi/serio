#include "common.h"

#ifdef SERIO_TEST_BOOST

#include <serio/boost.h>

#include <boost/fusion/include/at_key.hpp>
#include <boost/optional/optional_io.hpp>

// ---- boost::array ----

TEST(BoostArray, Int5)
{
    boost::array<int, 5> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostArray, Doubles)
{
    boost::array<double, 4> v{1.1, 2.2, 3.3, 4.4};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostArray, SingleElement)
{
    boost::array<int, 1> v{99};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostArray, Floats)
{
    boost::array<float, 3> v{1.0f, 2.0f, 3.0f};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostArray, Bools)
{
    boost::array<bool, 4> v{true, false, true, false};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostArray, NoLengthPrefix)
{
    // Fixed-size array serializes without a length prefix
    boost::array<int, 3> arr{1, 2, 3};
    boost::container::vector<int> vec{1, 2, 3};
    auto arr_bytes = Serio::serialize<Serio::Binary>({}, arr);
    auto vec_bytes = Serio::serialize<Serio::Binary>({}, vec);
    EXPECT_NE(arr_bytes, vec_bytes);
}

TEST(BoostArray, Large)
{
    boost::array<int, 64> v{};
    for (int i = 0; i < 64; ++i) v[i] = i * 3;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostArray, JsonRoundtrip)
{
    boost::array<int, 3> v{10, 20, 30};
    EXPECT_EQ(roundtrip_json(v), v);
}

TEST(BoostArray, XmlRoundtrip)
{
    boost::array<double, 3> v{1.0, 2.0, 3.0};
    EXPECT_EQ(roundtrip_xml(v), v);
}

// ---- boost::container::vector ----

TEST(BoostContainerVector, Empty)
{
    boost::container::vector<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerVector, Ints)
{
    boost::container::vector<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerVector, Strings)
{
    boost::container::vector<std::string> v{"alpha", "beta", "gamma"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerVector, Large)
{
    boost::container::vector<double> v;
    for (int i = 0; i < 1000; ++i) v.push_back(i * 0.5);
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerVector, Nested)
{
    boost::container::vector<boost::container::vector<int>> v{{1, 2}, {3, 4, 5}, {}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerVector, JsonRoundtrip)
{
    boost::container::vector<int> v{10, 20, 30};
    EXPECT_EQ(roundtrip_json(v), v);
}

// ---- boost::container::deque ----

TEST(BoostContainerDeque, Empty)
{
    boost::container::deque<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerDeque, Ints)
{
    boost::container::deque<int> v{10, 20, 30, 40};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerDeque, Strings)
{
    boost::container::deque<std::string> v{"x", "y", "z"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::devector ----

TEST(BoostContainerDevector, Empty)
{
    boost::container::devector<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerDevector, Ints)
{
    boost::container::devector<int> v{5, 4, 3, 2, 1};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerDevector, Doubles)
{
    boost::container::devector<double> v{1.1, 2.2, 3.3};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::list ----

TEST(BoostContainerList, Empty)
{
    boost::container::list<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerList, Ints)
{
    boost::container::list<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerList, Strings)
{
    boost::container::list<std::string> v{"a", "bb", "ccc"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::slist ----

TEST(BoostContainerSlist, Empty)
{
    boost::container::slist<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerSlist, Ints)
{
    boost::container::slist<int> v{1, 2, 3};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::container::slist<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(BoostContainerSlist, Strings)
{
    boost::container::slist<std::string> v{"hello", "world"};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::container::slist<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- boost::container::small_vector ----

TEST(BoostContainerSmallVector, Empty)
{
    boost::container::small_vector<int, 8> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerSmallVector, WithinInlineCapacity)
{
    boost::container::small_vector<int, 8> v{1, 2, 3, 4};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerSmallVector, ExceedsInlineCapacity)
{
    boost::container::small_vector<int, 4> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::stable_vector ----

TEST(BoostContainerStableVector, Empty)
{
    boost::container::stable_vector<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerStableVector, Ints)
{
    boost::container::stable_vector<int> v{10, 20, 30};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerStableVector, Strings)
{
    boost::container::stable_vector<std::string> v{"p", "q", "r"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::static_vector ----

TEST(BoostContainerStaticVector, Empty)
{
    boost::container::static_vector<int, 16> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerStaticVector, Ints)
{
    boost::container::static_vector<int, 16> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerStaticVector, Floats)
{
    boost::container::static_vector<float, 10> v{1.0f, 2.0f, 3.0f};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::string ----

TEST(BoostContainerString, Empty)
{
    boost::container::string v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerString, Simple)
{
    boost::container::string v("hello world");
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerString, Long)
{
    boost::container::string v(1000, 'x');
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerString, JsonRoundtrip)
{
    boost::container::string v("json test");
    EXPECT_EQ(roundtrip_json(v), v);
}

// ---- boost::container::set ----

TEST(BoostContainerSet, Empty)
{
    boost::container::set<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerSet, Ints)
{
    boost::container::set<int> v{3, 1, 4, 1, 5, 9};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerSet, Strings)
{
    boost::container::set<std::string> v{"banana", "apple", "cherry"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::map ----

TEST(BoostContainerMap, Empty)
{
    boost::container::map<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerMap, IntToInt)
{
    boost::container::map<int, int> v{{1, 10}, {2, 20}, {3, 30}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerMap, StringToInt)
{
    boost::container::map<std::string, int> v{{"a", 1}, {"b", 2}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerMap, JsonRoundtrip)
{
    boost::container::map<std::string, int> v{{"x", 1}, {"y", 2}};
    EXPECT_EQ(roundtrip_json(v), v);
}

// ---- boost::container::multiset ----

TEST(BoostContainerMultiset, Empty)
{
    boost::container::multiset<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerMultiset, WithDuplicates)
{
    boost::container::multiset<int> v{1, 1, 2, 2, 3};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::multimap ----

TEST(BoostContainerMultimap, Empty)
{
    boost::container::multimap<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerMultimap, WithDuplicateKeys)
{
    boost::container::multimap<int, std::string> v{{1, "a"}, {1, "b"}, {2, "c"}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::container::multimap<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), v.size());
    EXPECT_EQ(out.count(1), 2u);
    EXPECT_EQ(out.count(2), 1u);
}

// ---- boost::container::flat_set ----

TEST(BoostContainerFlatSet, Empty)
{
    boost::container::flat_set<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerFlatSet, Ints)
{
    boost::container::flat_set<int> v{5, 3, 1, 4, 2};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerFlatSet, Strings)
{
    boost::container::flat_set<std::string> v{"z", "y", "x"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::flat_map ----

TEST(BoostContainerFlatMap, Empty)
{
    boost::container::flat_map<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerFlatMap, IntToInt)
{
    boost::container::flat_map<int, int> v{{1, 100}, {2, 200}, {3, 300}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerFlatMap, StringToString)
{
    boost::container::flat_map<std::string, std::string> v{{"hello", "world"}, {"foo", "bar"}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::flat_multiset ----

TEST(BoostContainerFlatMultiset, Empty)
{
    boost::container::flat_multiset<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerFlatMultiset, WithDuplicates)
{
    boost::container::flat_multiset<int> v{1, 1, 2, 3, 3};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::container::flat_multimap ----

TEST(BoostContainerFlatMultimap, Empty)
{
    boost::container::flat_multimap<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostContainerFlatMultimap, WithDuplicateKeys)
{
    boost::container::flat_multimap<int, std::string> v{{1, "a"}, {1, "b"}, {2, "c"}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::container::flat_multimap<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.size(), v.size());
    EXPECT_EQ(out.count(1), 2u);
}

// ---- boost::unordered_set ----

TEST(BoostUnorderedSet, Empty)
{
    boost::unordered_set<int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostUnorderedSet, Ints)
{
    boost::unordered_set<int> v{1, 2, 3, 4, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostUnorderedSet, Strings)
{
    boost::unordered_set<std::string> v{"a", "b", "c"};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::unordered_map ----

TEST(BoostUnorderedMap, Empty)
{
    boost::unordered_map<int, int> v;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostUnorderedMap, IntToInt)
{
    boost::unordered_map<int, int> v{{1, 100}, {2, 200}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BoostUnorderedMap, StringToString)
{
    boost::unordered_map<std::string, std::string> v{{"key", "val"}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- boost::unordered_multiset ----

TEST(BoostUnorderedMultiset, Empty)
{
    boost::unordered_multiset<int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::unordered_multiset<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(BoostUnorderedMultiset, WithDuplicates)
{
    boost::unordered_multiset<int> v{1, 1, 2, 3, 3};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::unordered_multiset<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- boost::unordered_multimap ----

TEST(BoostUnorderedMultimap, Empty)
{
    boost::unordered_multimap<int, int> v;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::unordered_multimap<int, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

TEST(BoostUnorderedMultimap, WithDuplicateKeys)
{
    boost::unordered_multimap<std::string, int> v{{"a", 1}, {"a", 2}, {"b", 3}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::unordered_multimap<std::string, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, v);
}

// ---- boost::optional ----

TEST(BoostOptional, EmptyInt)
{
    boost::optional<int> o;
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BoostOptional, SetInt)
{
    boost::optional<int> o{42};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BoostOptional, ZeroInt)
{
    boost::optional<int> o{0};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BoostOptional, EmptyVsSet)
{
    boost::optional<int> empty;
    boost::optional<int> set{42};
    auto b1 = Serio::serialize<Serio::Binary>({}, empty);
    auto b2 = Serio::serialize<Serio::Binary>({}, set);
    EXPECT_NE(b1, b2);
}

TEST(BoostOptional, SetString)
{
    boost::optional<std::string> o{"hello"};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BoostOptional, EmptyString)
{
    boost::optional<std::string> o;
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BoostOptional, CustomType)
{
    boost::optional<Point2D> o{Point2D{1.0f, 2.0f}};
    auto bytes = Serio::serialize<Serio::Binary>({}, o);
    boost::optional<Point2D> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(*out, (Point2D{1.0f, 2.0f}));
}

TEST(BoostOptional, JsonRoundtrip)
{
    boost::optional<int> o{99};
    EXPECT_EQ(roundtrip_json(o), o);
    o.reset();
    EXPECT_EQ(roundtrip_json(o), o);
}

TEST(BoostOptional, XmlRoundtrip)
{
    boost::optional<std::string> o{"xml"};
    EXPECT_EQ(roundtrip_xml(o), o);
}

// ---- boost::variant ----

TEST(BoostVariant, FirstAlternative)
{
    boost::variant<int, std::string, double> v{42};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::variant<int, std::string, double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.which(), 0);
    EXPECT_EQ(boost::get<int>(out), 42);
}

TEST(BoostVariant, SecondAlternative)
{
    boost::variant<int, std::string, double> v{std::string("hello")};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::variant<int, std::string, double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.which(), 1);
    EXPECT_EQ(boost::get<std::string>(out), "hello");
}

TEST(BoostVariant, ThirdAlternative)
{
    boost::variant<int, std::string, double> v{3.14};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::variant<int, std::string, double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.which(), 2);
    EXPECT_DOUBLE_EQ(boost::get<double>(out), 3.14);
}

TEST(BoostVariant, SingleType)
{
    boost::variant<int> v{99};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::variant<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(boost::get<int>(out), 99);
}

TEST(BoostVariant, JsonRoundtrip)
{
    boost::variant<int, std::string> v{std::string("world")};
    auto bytes = Serio::serialize<Serio::JSON>({}, v);
    boost::variant<int, std::string> out;
    Serio::deserialize<Serio::JSON>({}, bytes, out);
    ASSERT_EQ(out.which(), 1);
    EXPECT_EQ(boost::get<std::string>(out), "world");
}

TEST(BoostVariant, XmlRoundtrip)
{
    boost::variant<int, std::string> v{123};
    auto bytes = Serio::serialize<Serio::XML>({}, v);
    boost::variant<int, std::string> out{std::string("")};
    Serio::deserialize<Serio::XML>({}, bytes, out);
    ASSERT_EQ(out.which(), 0);
    EXPECT_EQ(boost::get<int>(out), 123);
}

// ---- boost::tuple ----

TEST(BoostTuple, OneElement)
{
    boost::tuple<int> t{42};
    auto bytes = Serio::serialize<Serio::Binary>({}, t);
    boost::tuple<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(boost::get<0>(out), 42);
}

TEST(BoostTuple, TwoElements)
{
    boost::tuple<int, std::string> t{1, "hello"};
    auto bytes = Serio::serialize<Serio::Binary>({}, t);
    boost::tuple<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(boost::get<0>(out), 1);
    EXPECT_EQ(boost::get<1>(out), "hello");
}

TEST(BoostTuple, ThreeElements)
{
    boost::tuple<bool, int, double> t{true, 42, 3.14};
    auto bytes = Serio::serialize<Serio::Binary>({}, t);
    boost::tuple<bool, int, double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(boost::get<0>(out), true);
    EXPECT_EQ(boost::get<1>(out), 42);
    EXPECT_DOUBLE_EQ(boost::get<2>(out), 3.14);
}

TEST(BoostTuple, FourElements)
{
    boost::tuple<int, float, std::string, bool> t{1, 2.5f, "hi", false};
    auto bytes = Serio::serialize<Serio::Binary>({}, t);
    boost::tuple<int, float, std::string, bool> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(boost::get<0>(out), 1);
    EXPECT_FLOAT_EQ(boost::get<1>(out), 2.5f);
    EXPECT_EQ(boost::get<2>(out), "hi");
    EXPECT_EQ(boost::get<3>(out), false);
}

TEST(BoostTuple, JsonRoundtrip)
{
    boost::tuple<int, std::string> t{7, "seven"};
    auto json = Serio::serialize<Serio::JSON>({}, t);
    boost::tuple<int, std::string> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(boost::get<0>(out), 7);
    EXPECT_EQ(boost::get<1>(out), "seven");
}

TEST(BoostTuple, XmlRoundtrip)
{
    boost::tuple<int, double> t{3, 2.718};
    auto xml = Serio::serialize<Serio::XML>({}, t);
    boost::tuple<int, double> out;
    Serio::deserialize<Serio::XML>({}, xml, out);
    EXPECT_EQ(boost::get<0>(out), 3);
    EXPECT_NEAR(boost::get<1>(out), 2.718, 1e-6);
}

// ---- boost::fusion::nil_ ----

TEST(BoostFusionNil, Roundtrip)
{
    boost::fusion::nil_ v{};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::fusion::nil_ out{};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
}

TEST(BoostFusionNil, JsonRoundtrip)
{
    boost::fusion::nil_ v{};
    auto json = Serio::serialize<Serio::JSON>({}, v);
    boost::fusion::nil_ out{};
    Serio::deserialize<Serio::JSON>({}, json, out);
}

// ---- boost::fusion::pair ----

TEST(BoostFusionPair, StringValue)
{
    auto v = boost::fusion::make_pair<int>(std::string("hello"));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::fusion::pair<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.second, "hello");
}

TEST(BoostFusionPair, IntValue)
{
    auto v = boost::fusion::make_pair<std::string>(42);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::fusion::pair<std::string, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.second, 42);
}

TEST(BoostFusionPair, EmptyStringValue)
{
    auto v = boost::fusion::make_pair<int>(std::string(""));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::fusion::pair<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.second, "");
}

TEST(BoostFusionPair, JsonRoundtrip)
{
    boost::fusion::pair<int, double> p(3.14);
    auto json = Serio::serialize<Serio::JSON>({}, p);
    boost::fusion::pair<int, double> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_DOUBLE_EQ(out.second, 3.14);
}

// ---- boost::fusion::set ----

TEST(BoostFusionSet, TwoTypes)
{
    boost::fusion::set<int, std::string> v;
    boost::fusion::at_key<int>(v) = 42;
    boost::fusion::at_key<std::string>(v) = "hello";

    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::fusion::set<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);

    EXPECT_EQ(boost::fusion::at_key<int>(out), 42);
    EXPECT_EQ(boost::fusion::at_key<std::string>(out), "hello");
}

TEST(BoostFusionSet, ThreeTypes)
{
    boost::fusion::set<int, double, std::string> v;
    boost::fusion::at_key<int>(v) = 1;
    boost::fusion::at_key<double>(v) = 2.5;
    boost::fusion::at_key<std::string>(v) = "three";

    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::fusion::set<int, double, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);

    EXPECT_EQ(boost::fusion::at_key<int>(out), 1);
    EXPECT_DOUBLE_EQ(boost::fusion::at_key<double>(out), 2.5);
    EXPECT_EQ(boost::fusion::at_key<std::string>(out), "three");
}

TEST(BoostFusionSet, JsonRoundtrip)
{
    boost::fusion::set<int, std::string> v;
    boost::fusion::at_key<int>(v) = 99;
    boost::fusion::at_key<std::string>(v) = "json";

    auto json = Serio::serialize<Serio::JSON>({}, v);
    boost::fusion::set<int, std::string> out;
    Serio::deserialize<Serio::JSON>({}, json, out);

    EXPECT_EQ(boost::fusion::at_key<int>(out), 99);
    EXPECT_EQ(boost::fusion::at_key<std::string>(out), "json");
}

// ---- boost::fusion::map ----

TEST(BoostFusionMap, OneEntry)
{
    boost::fusion::map<boost::fusion::pair<int, std::string>> v;
    boost::fusion::at_key<int>(v) = "value";

    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    boost::fusion::map<boost::fusion::pair<int, std::string>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);

    EXPECT_EQ(boost::fusion::at_key<int>(out), "value");
}

TEST(BoostFusionMap, TwoEntries)
{
    using Map = boost::fusion::map<boost::fusion::pair<int, std::string>, boost::fusion::pair<double, int>>;

    Map v;
    boost::fusion::at_key<int>(v) = "hello";
    boost::fusion::at_key<double>(v) = 99;

    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    Map out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);

    EXPECT_EQ(boost::fusion::at_key<int>(out), "hello");
    EXPECT_EQ(boost::fusion::at_key<double>(out), 99);
}

TEST(BoostFusionMap, JsonRoundtrip)
{
    boost::fusion::map<boost::fusion::pair<int, double>> v;
    boost::fusion::at_key<int>(v) = 3.14;

    auto json = Serio::serialize<Serio::JSON>({}, v);
    boost::fusion::map<boost::fusion::pair<int, double>> out;
    Serio::deserialize<Serio::JSON>({}, json, out);

    EXPECT_DOUBLE_EQ(boost::fusion::at_key<int>(out), 3.14);
}

// ---- boost::fusion::cons ----

TEST(BoostFusionCons, SingleElement)
{
    using Cons = boost::fusion::cons<int, boost::fusion::nil_>;
    Cons v(42, boost::fusion::nil_());
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    Cons out(0, boost::fusion::nil_());
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.car, 42);
}

TEST(BoostFusionCons, TwoElements)
{
    using Inner = boost::fusion::cons<std::string, boost::fusion::nil_>;
    using Outer = boost::fusion::cons<int, Inner>;
    Outer v(42, Inner(std::string("hello"), boost::fusion::nil_()));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    Outer out(0, Inner(std::string(""), boost::fusion::nil_()));
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.car, 42);
    EXPECT_EQ(out.cdr.car, std::string("hello"));
}

TEST(BoostFusionCons, ThreeElements)
{
    using C3 = boost::fusion::cons<double, boost::fusion::nil_>;
    using C2 = boost::fusion::cons<std::string, C3>;
    using C1 = boost::fusion::cons<int, C2>;
    C1 v(1, C2(std::string("two"), C3(3.0, boost::fusion::nil_())));
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    C1 out(0, C2(std::string(""), C3(0.0, boost::fusion::nil_())));
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.car, 1);
    EXPECT_EQ(out.cdr.car, std::string("two"));
    EXPECT_DOUBLE_EQ(out.cdr.cdr.car, 3.0);
}

TEST(BoostFusionCons, JsonSingleElement)
{
    using Cons = boost::fusion::cons<int, boost::fusion::nil_>;
    Cons v(7, boost::fusion::nil_());
    auto json = Serio::serialize<Serio::JSON>({}, v);
    Cons out(0, boost::fusion::nil_());
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out.car, 7);
}

TEST(BoostFusionCons, JsonTwoElements)
{
    using Inner = boost::fusion::cons<std::string, boost::fusion::nil_>;
    using Outer = boost::fusion::cons<int, Inner>;
    Outer v(7, Inner(std::string("json"), boost::fusion::nil_()));
    auto json = Serio::serialize<Serio::JSON>({}, v);
    Outer out(0, Inner(std::string(""), boost::fusion::nil_()));
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(out.car, 7);
    EXPECT_EQ(out.cdr.car, std::string("json"));
}

#endif  // SERIO_TEST_BOOST
