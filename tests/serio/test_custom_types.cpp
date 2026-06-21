#include <string>
#include <vector>

#include "common.h"

// ---- SERIO_REGISTER (types defined in common.h) ----

TEST(CustomTypes, Point2DRoundtrip)
{
    Point2D p{1.5f, -2.5f};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(CustomTypes, Point2DZero)
{
    Point2D p{0.0f, 0.0f};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(CustomTypes, Point2DAllFormats) { check(Point2D{1.0f, 2.0f}); }

TEST(CustomTypes, Point3DRoundtrip)
{
    Point3D p{1.0, 2.0, 3.0};
    EXPECT_EQ(roundtrip_binary(p), p);
}

TEST(CustomTypes, NamedRoundtrip)
{
    Named n{"hello", 42};
    EXPECT_EQ(roundtrip_binary(n), n);
}

TEST(CustomTypes, NamedEmptyString)
{
    Named n{"", -1};
    EXPECT_EQ(roundtrip_binary(n), n);
}

TEST(CustomTypes, NamedAllFormats) { check(Named{"world", 99}); }

TEST(CustomTypes, NestedStruct)
{
    Nested n{{1.0f, 2.0f}, {1, 2, 3}, "label"};
    EXPECT_EQ(roundtrip_binary(n), n);
}

TEST(CustomTypes, NestedStructEmptyData)
{
    Nested n{{0.0f, 0.0f}, {}, ""};
    EXPECT_EQ(roundtrip_binary(n), n);
}

TEST(CustomTypes, AllBuiltinsRoundtrip)
{
    AllBuiltins a{true,  -1,         255, -1000, 60000, -1000000, 3000000000u, -9000000000LL, 18000000000000000000ULL,
                  3.14f, 2.718281828};
    EXPECT_EQ(roundtrip_binary(a), a);
}

TEST(CustomTypes, VectorOfCustom)
{
    std::vector<Point2D> v{{0.0f, 0.0f}, {1.0f, 2.0f}, {-3.14f, 100.0f}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(CustomTypes, MapOfCustom)
{
    std::map<std::string, Point2D> m{{"origin", {0.0f, 0.0f}}, {"unit", {1.0f, 1.0f}}};
    EXPECT_EQ(roundtrip_binary(m), m);
}

TEST(CustomTypes, CustomInOptional)
{
    std::optional<Point2D> o{Point2D{1.0f, 2.0f}};
    EXPECT_EQ(roundtrip_binary(o), o);
    o = std::nullopt;
    EXPECT_EQ(roundtrip_binary(o), o);
}

TEST(CustomTypes, CustomInVariant)
{
    std::variant<int, Point2D, std::string> v{Point2D{3.0f, 4.0f}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(CustomTypes, CustomInSharedPtr)
{
    auto p = std::make_shared<Named>(Named{"test", 99});
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<Named> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    Named expected{"test", 99};
    EXPECT_EQ(*out, expected);
}

TEST(CustomTypes, MultipleCustomValues)
{
    Point2D p{1.0f, 2.0f};
    Named n{"test", 5};
    Vec3 v{1.0f, 2.0f, 3.0f};
    auto bytes = Serio::serialize<Serio::Binary>({}, p, n, v);
    Point2D op;
    Named on;
    Vec3 ov;
    Serio::deserialize<Serio::Binary>({}, bytes, op, on, ov);
    EXPECT_EQ(op, p);
    EXPECT_EQ(on, n);
    EXPECT_EQ(ov, v);
}

TEST(CustomTypes, Point2DJson) { EXPECT_EQ(roundtrip_json(Point2D{1.0f, 2.0f}), (Point2D{1.0f, 2.0f})); }
TEST(CustomTypes, Vec3Json) { EXPECT_EQ(roundtrip_json(Vec3{1.0f, 2.0f, 3.0f}), (Vec3{1.0f, 2.0f, 3.0f})); }
TEST(CustomTypes, Point2DXml) { EXPECT_EQ(roundtrip_xml(Point2D{3.0f, 4.0f}), (Point2D{3.0f, 4.0f})); }

// ---- Serio::CustomClass (Vec3 — external type, no SERIO_REGISTER) ----

TEST(CustomClass, Vec3Roundtrip) { EXPECT_EQ(roundtrip_binary(Vec3{1.0f, 2.0f, 3.0f}), (Vec3{1.0f, 2.0f, 3.0f})); }
TEST(CustomClass, Vec3Zero) { EXPECT_EQ(roundtrip_binary(Vec3{0.0f, 0.0f, 0.0f}), (Vec3{0.0f, 0.0f, 0.0f})); }
TEST(CustomClass, Vec3Negative) { EXPECT_EQ(roundtrip_binary(Vec3{-1.0f, -2.0f, -3.0f}), (Vec3{-1.0f, -2.0f, -3.0f})); }

TEST(CustomClass, Vec3InVector)
{
    std::vector<Vec3> vs{{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
    auto bytes = Serio::serialize<Serio::Binary>({}, vs);
    std::vector<Vec3> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), vs.size());
    for (std::size_t i = 0; i < vs.size(); ++i) EXPECT_EQ(out[i], vs[i]);
}

TEST(CustomClass, Vec3InMap)
{
    std::map<std::string, Vec3> m{{"i", {1.0f, 0.0f, 0.0f}}, {"j", {0.0f, 1.0f, 0.0f}}};
    auto bytes = Serio::serialize<Serio::Binary>({}, m);
    std::map<std::string, Vec3> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 2u);
    EXPECT_EQ(out["i"], (Vec3{1.0f, 0.0f, 0.0f}));
    EXPECT_EQ(out["j"], (Vec3{0.0f, 1.0f, 0.0f}));
}

// ---- IntBag (SERIO_FRIEND with manual size-prefix, binary-only) ----

TEST(CustomContainer, IntBagEmpty) { check_bin(IntBag{}); }
TEST(CustomContainer, IntBagSimple) { check_bin(IntBag{{1, 2, 3, 4, 5}}); }
TEST(CustomContainer, IntBagSingle) { check_bin(IntBag{{42}}); }

TEST(CustomContainer, IntBagLarge)
{
    std::vector<int> data(2000);
    for (int i = 0; i < 2000; ++i) data[i] = i * 7;
    check_bin(IntBag{data});
}

TEST(CustomContainer, IntBagInVector)
{
    std::vector<IntBag> v{IntBag{{1, 2}}, IntBag{}, IntBag{{3, 4, 5}}};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::vector<IntBag> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), v.size());
    for (std::size_t i = 0; i < v.size(); ++i) EXPECT_EQ(out[i], v[i]);
}

// ---- Locally defined types (not in common.h) ----

struct Point
{
    float x, y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    SERIO_REGISTER(x, y)
};

TEST(LocalCustomClass, PointDefault) { check(Point{0.0f, 0.0f}); }
TEST(LocalCustomClass, PointBasic) { check(Point{1.0f, 2.0f}); }
TEST(LocalCustomClass, PointNegative) { check(Point{-5.0f, 3.5f}); }

struct Line
{
    Point start, end;
    bool operator==(const Line& o) const { return start == o.start && end == o.end; }
    SERIO_REGISTER(start, end)
};

TEST(LocalCustomClass, Line) { check(Line{{0.0f, 0.0f}, {1.0f, 1.0f}}); }
TEST(LocalCustomClass, LineOrigin) { check(Line{{-1.0f, -1.0f}, {1.0f, 1.0f}}); }

struct Packet
{
    uint32_t id;
    std::string name;
    std::vector<int> data;
    bool valid;

    bool operator==(const Packet& o) const
    {
        return id == o.id && name == o.name && data == o.data && valid == o.valid;
    }
    SERIO_REGISTER(id, name, data, valid)
};

TEST(LocalCustomClass, PacketEmpty) { check(Packet{0, "", {}, false}); }
TEST(LocalCustomClass, PacketFull) { check(Packet{42, "hello", {1, 2, 3}, true}); }
TEST(LocalCustomClass, VectorOfPackets)
{
    std::vector<Packet> v{
        {1, "a", {10}, true},
        {2, "b", {}, false},
        {3, "cc", {1, 2, 3, 4}, true},
    };
    check(v);
}

struct Shape
{
    std::string color;
    bool operator==(const Shape& o) const { return color == o.color; }
    SERIO_REGISTER(color)
};

struct Circle : Shape
{
    float radius;
    bool operator==(const Circle& o) const { return color == o.color && radius == o.radius; }
    SERIO_REGISTER(color, radius)
};

TEST(LocalCustomClass, DerivedCircle) { check(Circle{{"red"}, 5.0f}); }
TEST(LocalCustomClass, DerivedCircleDefault) { check(Circle{{"blue"}, 0.0f}); }
