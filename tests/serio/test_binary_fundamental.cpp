#include "common.h"

enum Color
{
    Red = 0,
    Green = 1,
    Blue = 2
};
enum class Direction : uint8_t
{
    North = 0,
    South = 1,
    East = 2,
    West = 3
};

// ---- bool ----

TEST(BinaryFundamental, BoolFalse) { EXPECT_EQ(roundtrip_binary(false), false); }
TEST(BinaryFundamental, BoolTrue) { EXPECT_EQ(roundtrip_binary(true), true); }

TEST(BinaryFundamental, BoolMultiple)
{
    auto bytes = Serio::serialize<Serio::Binary>({}, true, false, true, true, false);
    bool a, b, c, d, e;
    Serio::deserialize<Serio::Binary>({}, bytes, a, b, c, d, e);
    EXPECT_TRUE(a);
    EXPECT_FALSE(b);
    EXPECT_TRUE(c);
    EXPECT_TRUE(d);
    EXPECT_FALSE(e);
}

// ---- char ----

TEST(BinaryFundamental, CharA) { EXPECT_EQ(roundtrip_binary('A'), 'A'); }
TEST(BinaryFundamental, CharNul) { EXPECT_EQ(roundtrip_binary('\0'), '\0'); }
TEST(BinaryFundamental, CharMax)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<char>::max()), std::numeric_limits<char>::max());
}
TEST(BinaryFundamental, CharMin)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<char>::min()), std::numeric_limits<char>::min());
}

// ---- signed integers ----

TEST(BinaryFundamental, Int8Zero) { EXPECT_EQ(roundtrip_binary(int8_t(0)), int8_t(0)); }
TEST(BinaryFundamental, Int8Max)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int8_t>::max()), std::numeric_limits<int8_t>::max());
}
TEST(BinaryFundamental, Int8Min)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int8_t>::min()), std::numeric_limits<int8_t>::min());
}
TEST(BinaryFundamental, Int8Neg) { EXPECT_EQ(roundtrip_binary(int8_t(-42)), int8_t(-42)); }

TEST(BinaryFundamental, Int16Zero) { EXPECT_EQ(roundtrip_binary(int16_t(0)), int16_t(0)); }
TEST(BinaryFundamental, Int16Max)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int16_t>::max()), std::numeric_limits<int16_t>::max());
}
TEST(BinaryFundamental, Int16Min)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int16_t>::min()), std::numeric_limits<int16_t>::min());
}
TEST(BinaryFundamental, Int16Neg) { EXPECT_EQ(roundtrip_binary(int16_t(-1000)), int16_t(-1000)); }

TEST(BinaryFundamental, Int32Zero) { EXPECT_EQ(roundtrip_binary(int32_t(0)), int32_t(0)); }
TEST(BinaryFundamental, Int32Max)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int32_t>::max()), std::numeric_limits<int32_t>::max());
}
TEST(BinaryFundamental, Int32Min)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int32_t>::min()), std::numeric_limits<int32_t>::min());
}
TEST(BinaryFundamental, Int32Typical) { EXPECT_EQ(roundtrip_binary(int32_t(123456789)), int32_t(123456789)); }
TEST(BinaryFundamental, Int32NegTyp) { EXPECT_EQ(roundtrip_binary(int32_t(-123456789)), int32_t(-123456789)); }

TEST(BinaryFundamental, Int64Zero) { EXPECT_EQ(roundtrip_binary(int64_t(0)), int64_t(0)); }
TEST(BinaryFundamental, Int64Max)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int64_t>::max()), std::numeric_limits<int64_t>::max());
}
TEST(BinaryFundamental, Int64Min)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<int64_t>::min()), std::numeric_limits<int64_t>::min());
}
TEST(BinaryFundamental, Int64Large) { EXPECT_EQ(roundtrip_binary(int64_t(9000000000LL)), int64_t(9000000000LL)); }

// ---- unsigned integers ----

TEST(BinaryFundamental, Uint8Zero) { EXPECT_EQ(roundtrip_binary(uint8_t(0)), uint8_t(0)); }
TEST(BinaryFundamental, Uint8Max) { EXPECT_EQ(roundtrip_binary(uint8_t(255)), uint8_t(255)); }
TEST(BinaryFundamental, Uint8Mid) { EXPECT_EQ(roundtrip_binary(uint8_t(128)), uint8_t(128)); }

TEST(BinaryFundamental, Uint16Zero) { EXPECT_EQ(roundtrip_binary(uint16_t(0)), uint16_t(0)); }
TEST(BinaryFundamental, Uint16Max)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<uint16_t>::max()), std::numeric_limits<uint16_t>::max());
}

TEST(BinaryFundamental, Uint32Zero) { EXPECT_EQ(roundtrip_binary(uint32_t(0)), uint32_t(0)); }
TEST(BinaryFundamental, Uint32Max)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<uint32_t>::max()), std::numeric_limits<uint32_t>::max());
}

TEST(BinaryFundamental, Uint64Zero) { EXPECT_EQ(roundtrip_binary(uint64_t(0)), uint64_t(0)); }
TEST(BinaryFundamental, Uint64Max)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<uint64_t>::max()), std::numeric_limits<uint64_t>::max());
}
TEST(BinaryFundamental, Uint64Large)
{
    EXPECT_EQ(roundtrip_binary(uint64_t(18000000000000000000ULL)), uint64_t(18000000000000000000ULL));
}

// ---- float ----

TEST(BinaryFundamental, FloatZero) { EXPECT_EQ(roundtrip_binary(0.0f), 0.0f); }
TEST(BinaryFundamental, FloatOne) { EXPECT_EQ(roundtrip_binary(1.0f), 1.0f); }
TEST(BinaryFundamental, FloatNeg) { EXPECT_EQ(roundtrip_binary(-3.14f), -3.14f); }
TEST(BinaryFundamental, FloatMax)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<float>::max()), std::numeric_limits<float>::max());
}
TEST(BinaryFundamental, FloatMin)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<float>::lowest()), std::numeric_limits<float>::lowest());
}
TEST(BinaryFundamental, FloatInf)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<float>::infinity()), std::numeric_limits<float>::infinity());
}
TEST(BinaryFundamental, FloatNegInf)
{
    EXPECT_EQ(roundtrip_binary(-std::numeric_limits<float>::infinity()), -std::numeric_limits<float>::infinity());
}
TEST(BinaryFundamental, FloatNaN)
{
    float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_TRUE(std::isnan(roundtrip_binary(nan)));
}
TEST(BinaryFundamental, FloatSmall)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<float>::denorm_min()), std::numeric_limits<float>::denorm_min());
}
TEST(BinaryFundamental, FloatEpsilon)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<float>::epsilon()), std::numeric_limits<float>::epsilon());
}

// ---- double ----

TEST(BinaryFundamental, DoubleZero) { EXPECT_EQ(roundtrip_binary(0.0), 0.0); }
TEST(BinaryFundamental, DoubleOne) { EXPECT_EQ(roundtrip_binary(1.0), 1.0); }
TEST(BinaryFundamental, DoubleNeg) { EXPECT_EQ(roundtrip_binary(-2.718281828), -2.718281828); }
TEST(BinaryFundamental, DoubleMax)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<double>::max()), std::numeric_limits<double>::max());
}
TEST(BinaryFundamental, DoubleMin)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<double>::lowest()), std::numeric_limits<double>::lowest());
}
TEST(BinaryFundamental, DoubleInf)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<double>::infinity()), std::numeric_limits<double>::infinity());
}
TEST(BinaryFundamental, DoubleNaN)
{
    double nan = std::numeric_limits<double>::quiet_NaN();
    EXPECT_TRUE(std::isnan(roundtrip_binary(nan)));
}
TEST(BinaryFundamental, DoubleEpsilon)
{
    EXPECT_EQ(roundtrip_binary(std::numeric_limits<double>::epsilon()), std::numeric_limits<double>::epsilon());
}
TEST(BinaryFundamental, DoublePi) { EXPECT_EQ(roundtrip_binary(3.14159265358979323846), 3.14159265358979323846); }

// ---- enum (unscoped) ----

TEST(BinaryFundamental, EnumRed) { EXPECT_EQ(roundtrip_binary(Red), Red); }
TEST(BinaryFundamental, EnumGreen) { EXPECT_EQ(roundtrip_binary(Green), Green); }
TEST(BinaryFundamental, EnumBlue) { EXPECT_EQ(roundtrip_binary(Blue), Blue); }

// ---- enum class ----

TEST(BinaryFundamental, EnumClassNorth) { EXPECT_EQ(roundtrip_binary(Direction::North), Direction::North); }
TEST(BinaryFundamental, EnumClassSouth) { EXPECT_EQ(roundtrip_binary(Direction::South), Direction::South); }
TEST(BinaryFundamental, EnumClassEast) { EXPECT_EQ(roundtrip_binary(Direction::East), Direction::East); }
TEST(BinaryFundamental, EnumClassWest) { EXPECT_EQ(roundtrip_binary(Direction::West), Direction::West); }

#ifdef __SIZEOF_INT128__
// ---- __int128 ----

TEST(BinaryFundamental, Int128Zero)
{
    __int128 val = 0;
    auto bytes = Serio::serialize<Serio::Binary>({}, val);
    __int128 out = 1;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(val, out);
}

TEST(BinaryFundamental, Int128Large)
{
    __int128 val = ((__int128)1 << 100) + 12345;
    auto bytes = Serio::serialize<Serio::Binary>({}, val);
    __int128 out = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(val, out);
}

TEST(BinaryFundamental, Int128Negative)
{
    __int128 val = -((__int128)1 << 100);
    auto bytes = Serio::serialize<Serio::Binary>({}, val);
    __int128 out = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(val, out);
}

TEST(BinaryFundamental, Uint128Max)
{
    unsigned __int128 val = ~(unsigned __int128)0;
    auto bytes = Serio::serialize<Serio::Binary>({}, val);
    unsigned __int128 out = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(val, out);
}
#endif

// ---- Multiple values in one call ----

TEST(BinaryFundamental, MultipleValues)
{
    auto bytes = Serio::serialize<Serio::Binary>({}, int32_t(42), 3.14f, true, uint64_t(999));
    int32_t i;
    float f;
    bool b;
    uint64_t u;
    Serio::deserialize<Serio::Binary>({}, bytes, i, f, b, u);
    EXPECT_EQ(i, 42);
    EXPECT_EQ(f, 3.14f);
    EXPECT_TRUE(b);
    EXPECT_EQ(u, 999u);
}

// ---- int (platform-dependent, still supported) ----

TEST(BinaryFundamental, IntNative)
{
    int v = -777777;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryFundamental, UnsignedIntNative)
{
    unsigned v = 4000000000u;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryFundamental, LongNative)
{
    long v = -123456789L;
    EXPECT_EQ(roundtrip_binary(v), v);
}

TEST(BinaryFundamental, UnsignedLongNative)
{
    unsigned long v = 9999999999UL;
    EXPECT_EQ(roundtrip_binary(v), v);
}
