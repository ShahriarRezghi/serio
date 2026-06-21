#include "common.h"

// ---- std::optional ----

TEST(BinaryOptional, EmptyInt)
{
    std::optional<int> o;
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, SetInt)
{
    std::optional<int> o{42};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, ZeroInt)
{
    std::optional<int> o{0};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, EmptyString)
{
    std::optional<std::string> o;
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, SetString)
{
    std::optional<std::string> o{"hello"};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, EmptyVsZero)
{
    std::optional<int> empty;
    std::optional<int> zero{0};
    auto b1 = Serio::serialize<Serio::Binary>({}, empty);
    auto b2 = Serio::serialize<Serio::Binary>({}, zero);
    EXPECT_NE(b1, b2);
}

TEST(BinaryOptional, CustomType)
{
    std::optional<Point2D> o{Point2D{1.0f, 2.0f}};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, EmptyCustomType)
{
    std::optional<Point2D> o;
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, VectorOfOptionals)
{
    std::vector<std::optional<int>> v{1, std::nullopt, 3, std::nullopt, 5};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryOptional, OptionalVector)
{
    std::optional<std::vector<int>> o{std::vector<int>{1, 2, 3}};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, OptionalEmptyVector)
{
    std::optional<std::vector<int>> o{std::vector<int>{}};
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(BinaryOptional, NestedOptional)
{
    std::optional<std::optional<int>> o{std::optional<int>{42}};
    EXPECT_EQ(roundtrip_binary(o), o);

    std::optional<std::optional<int>> inner_empty{std::optional<int>{}};
    EXPECT_EQ(roundtrip_binary(inner_empty), inner_empty);

    std::optional<std::optional<int>> outer_empty;
    EXPECT_EQ(roundtrip_binary(outer_empty), outer_empty);
}

TEST(BinaryOptional, Bool)
{
    std::optional<bool> t{true}, f{false}, e;
    EXPECT_EQ(roundtrip_binary(t), t);
    EXPECT_EQ(roundtrip_binary(f), f);
    EXPECT_EQ(roundtrip_binary(e), e);
}

// ---- std::variant ----

TEST(BinaryVariant, IntActive)
{
    std::variant<int, std::string, double> v{42};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, StringActive)
{
    std::variant<int, std::string, double> v{std::string("hello")};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, DoubleActive)
{
    std::variant<int, std::string, double> v{3.14};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, MonostateVariant)
{
    std::variant<std::monostate, int, std::string> v;  // monostate active (index 0)
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, MonostateVariantWithInt)
{
    std::variant<std::monostate, int, std::string> v{42};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, MonostateVariantWithString)
{
    std::variant<std::monostate, int, std::string> v{std::string("hi")};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, SingleType)
{
    std::variant<int> v{100};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, VectorActive)
{
    std::variant<int, std::vector<int>> v{std::vector<int>{1, 2, 3}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, VectorOfVariants)
{
    using V = std::variant<int, std::string, double>;
    std::vector<V> vec{V{1}, V{std::string("hi")}, V{3.14}, V{0}};
    EXPECT_EQ(roundtrip_binary(vec), vec);
}

TEST(BinaryVariant, InMap)
{
    using V = std::variant<int, std::string>;
    std::map<std::string, V> m{{"a", 1}, {"b", std::string("hello")}};
    EXPECT_EQ(roundtrip_binary(m), m);
}

TEST(BinaryVariant, VariantWithCustomType)
{
    std::variant<Point2D, Point3D> v{Point2D{1.0f, 2.0f}};
    EXPECT_EQ(roundtrip_binary(v), v);
    v = Point3D{1.0, 2.0, 3.0};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryVariant, IndexZeroOfThree)
{
    std::variant<int, double, std::string> v{42};
    EXPECT_EQ(v.index(), 0u);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::variant<int, double, std::string> out{std::string("placeholder")};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.index(), 0u);
    EXPECT_EQ(std::get<0>(out), 42);
}

TEST(BinaryVariant, IndexOneOfThree)
{
    std::variant<int, double, std::string> v{3.14};
    EXPECT_EQ(v.index(), 1u);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::variant<int, double, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.index(), 1u);
    EXPECT_DOUBLE_EQ(std::get<1>(out), 3.14);
}

TEST(BinaryVariant, IndexTwoOfThree)
{
    std::variant<int, double, std::string> v{std::string("hello")};
    EXPECT_EQ(v.index(), 2u);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::variant<int, double, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.index(), 2u);
    EXPECT_EQ(std::get<2>(out), "hello");
}

// ---- std::expected (C++23) — void value type is not supported by Serio ----

TEST(BinaryExpected, HasValue)
{
    std::expected<int, std::string> e{42};
    auto bytes = Serio::serialize<Serio::Binary>({}, e);
    std::expected<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(*out, 42);
}

TEST(BinaryExpected, HasError)
{
    std::expected<int, std::string> e{std::unexpected("oops")};
    auto bytes = Serio::serialize<Serio::Binary>({}, e);
    std::expected<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_FALSE(out.has_value());
    EXPECT_EQ(out.error(), "oops");
}

TEST(BinaryExpected, ValueVsError)
{
    std::expected<int, int> v{42};
    std::expected<int, int> e{std::unexpected(42)};
    auto bv = Serio::serialize<Serio::Binary>({}, v);
    auto be = Serio::serialize<Serio::Binary>({}, e);
    EXPECT_NE(bv, be);
}

TEST(BinaryExpected, WithVector)
{
    std::expected<std::vector<int>, std::string> e{std::vector<int>{1, 2, 3}};
    auto bytes = Serio::serialize<Serio::Binary>({}, e);
    std::expected<std::vector<int>, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(*out, (std::vector<int>{1, 2, 3}));
}

TEST(BinaryExpected, WithCustomError)
{
    std::expected<std::string, int> e{std::unexpected(-1)};
    auto bytes = Serio::serialize<Serio::Binary>({}, e);
    std::expected<std::string, int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_FALSE(out.has_value());
    EXPECT_EQ(out.error(), -1);
}
