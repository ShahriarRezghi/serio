#include <cfloat>
#include <cmath>
#include <cstdint>
#include <limits>

#include "common.h"

// ---- bool ----

TEST(Bool, False) { check(false); }
TEST(Bool, True) { check(true); }

// ---- char ----

TEST(Char, Zero) { check(char(0)); }
TEST(Char, A) { check('A'); }
TEST(Char, Lower) { check('z'); }
TEST(Char, Max) { check(char(127)); }

// ---- signed integers ----

TEST(SignedChar, Zero) { check(static_cast<signed char>(0)); }
TEST(SignedChar, Pos) { check(static_cast<signed char>(42)); }
TEST(SignedChar, Neg) { check(static_cast<signed char>(-42)); }
TEST(SignedChar, Min) { check(std::numeric_limits<signed char>::min()); }
TEST(SignedChar, Max) { check(std::numeric_limits<signed char>::max()); }

TEST(Short, Zero) { check(static_cast<short>(0)); }
TEST(Short, Pos) { check(static_cast<short>(1000)); }
TEST(Short, Neg) { check(static_cast<short>(-1000)); }
TEST(Short, Min) { check(std::numeric_limits<short>::min()); }
TEST(Short, Max) { check(std::numeric_limits<short>::max()); }

TEST(Int, Zero) { check(0); }
TEST(Int, One) { check(1); }
TEST(Int, NegOne) { check(-1); }
TEST(Int, Large) { check(1000000); }
TEST(Int, NegLarge) { check(-1000000); }
TEST(Int, Min) { check(std::numeric_limits<int>::min()); }
TEST(Int, Max) { check(std::numeric_limits<int>::max()); }

TEST(Long, Zero) { check(0L); }
TEST(Long, Min) { check(std::numeric_limits<long>::min()); }
TEST(Long, Max) { check(std::numeric_limits<long>::max()); }

TEST(LongLong, Zero) { check(0LL); }
TEST(LongLong, Min) { check(std::numeric_limits<long long>::min()); }
TEST(LongLong, Max) { check(std::numeric_limits<long long>::max()); }

// ---- unsigned integers ----

TEST(UnsignedChar, Zero) { check(static_cast<unsigned char>(0)); }
TEST(UnsignedChar, Max) { check(std::numeric_limits<unsigned char>::max()); }
TEST(UnsignedChar, Mid) { check(static_cast<unsigned char>(200)); }

TEST(UnsignedShort, Zero) { check(static_cast<unsigned short>(0)); }
TEST(UnsignedShort, Max) { check(std::numeric_limits<unsigned short>::max()); }

TEST(UnsignedInt, Zero) { check(0u); }
TEST(UnsignedInt, Max) { check(std::numeric_limits<unsigned int>::max()); }

TEST(UnsignedLong, Zero) { check(0ul); }
TEST(UnsignedLong, Max) { check(std::numeric_limits<unsigned long>::max()); }

TEST(UnsignedLongLong, Zero) { check(0ull); }
TEST(UnsignedLongLong, Max) { check(std::numeric_limits<unsigned long long>::max()); }

// ---- wide char and Unicode chars ----

TEST(Wchar, Zero) { check(wchar_t(0)); }
TEST(Wchar, Ascii) { check(wchar_t('A')); }

TEST(Char16, Zero) { check(char16_t(0)); }
TEST(Char16, Ascii) { check(char16_t('Z')); }
TEST(Char16, Max) { check(std::numeric_limits<char16_t>::max()); }

TEST(Char32, Zero) { check(char32_t(0)); }
TEST(Char32, Ascii) { check(char32_t('z')); }
TEST(Char32, Max) { check(std::numeric_limits<char32_t>::max()); }

// ---- fixed-width integers (machine-independent preferred) ----

TEST(FixedWidth, Int8)
{
    check(int8_t(-1));
    check(int8_t(0));
    check(std::numeric_limits<int8_t>::min());
    check(std::numeric_limits<int8_t>::max());
}
TEST(FixedWidth, Int16)
{
    check(int16_t(0));
    check(std::numeric_limits<int16_t>::min());
    check(std::numeric_limits<int16_t>::max());
}
TEST(FixedWidth, Int32)
{
    check(int32_t(0));
    check(std::numeric_limits<int32_t>::min());
    check(std::numeric_limits<int32_t>::max());
}
TEST(FixedWidth, Int64)
{
    check(int64_t(0));
    check(std::numeric_limits<int64_t>::min());
    check(std::numeric_limits<int64_t>::max());
}
TEST(FixedWidth, Uint8)
{
    check(uint8_t(0));
    check(std::numeric_limits<uint8_t>::max());
}
TEST(FixedWidth, Uint16)
{
    check(uint16_t(0));
    check(std::numeric_limits<uint16_t>::max());
}
TEST(FixedWidth, Uint32)
{
    check(uint32_t(0));
    check(std::numeric_limits<uint32_t>::max());
}
TEST(FixedWidth, Uint64)
{
    check(uint64_t(0));
    check(std::numeric_limits<uint64_t>::max());
}

// ---- float and double (exact values that survive decimal roundtrip) ----

TEST(Float, Zero) { check(0.0f); }
TEST(Float, One) { check(1.0f); }
TEST(Float, NegOne) { check(-1.0f); }
TEST(Float, Half) { check(0.5f); }
TEST(Float, Quarter) { check(0.25f); }

TEST(Double, Zero) { check(0.0); }
TEST(Double, One) { check(1.0); }
TEST(Double, NegOne) { check(-1.0); }
TEST(Double, Half) { check(0.5); }
TEST(Double, Large) { check(1.0e15); }

// Special float values - only binary backend (not valid JSON)
TEST(Float, NaN_Binary)
{
    float v = std::numeric_limits<float>::quiet_NaN();
    float r = mem_rt<Serio::Binary>(v);
    EXPECT_TRUE(std::isnan(r));
}

TEST(Float, Inf_Binary)
{
    float v = std::numeric_limits<float>::infinity();
    float r = mem_rt<Serio::Binary>(v);
    EXPECT_TRUE(std::isinf(r) && r > 0);
}

TEST(Float, NegInf_Binary)
{
    float v = -std::numeric_limits<float>::infinity();
    float r = mem_rt<Serio::Binary>(v);
    EXPECT_TRUE(std::isinf(r) && r < 0);
}

TEST(Double, NaN_Binary)
{
    double v = std::numeric_limits<double>::quiet_NaN();
    double r = mem_rt<Serio::Binary>(v);
    EXPECT_TRUE(std::isnan(r));
}

TEST(Double, Inf_Binary)
{
    double v = std::numeric_limits<double>::infinity();
    double r = mem_rt<Serio::Binary>(v);
    EXPECT_TRUE(std::isinf(r) && r > 0);
}

// Binary-only for min/max (may not be representable in JSON text)
TEST(Float, Min_Binary) { check_bin(FLT_MIN); }
TEST(Float, Max_Binary) { check_bin(FLT_MAX); }
TEST(Double, Min_Binary) { check_bin(DBL_MIN); }
TEST(Double, Max_Binary) { check_bin(DBL_MAX); }

// ---- enum ----

enum Color
{
    Red = 0,
    Green = 1,
    Blue = 255
};
enum class Direction : uint8_t
{
    North = 0,
    South = 1,
    East = 2,
    West = 255
};
enum class Status : int32_t
{
    Active = -1,
    Inactive = 0,
    Pending = 2147483647
};

TEST(Enum, Plain_Red) { check(Red); }
TEST(Enum, Plain_Green) { check(Green); }
TEST(Enum, Plain_Blue) { check(Blue); }

TEST(Enum, Class_North) { check(Direction::North); }
TEST(Enum, Class_South) { check(Direction::South); }
TEST(Enum, Class_West) { check(Direction::West); }

TEST(Enum, Class_Int32_Active) { check(Status::Active); }
TEST(Enum, Class_Int32_Inactive) { check(Status::Inactive); }
TEST(Enum, Class_Int32_Pending) { check(Status::Pending); }

// ---- 128-bit integers (compiler-extension, test if available) ----

#ifdef __SIZEOF_INT128__
TEST(Int128, Zero)
{
    __int128 v = 0;
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    __int128 out = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(v, out);
}

TEST(Int128, Positive)
{
    __int128 v = __int128(1) << 64;  // larger than uint64_t max
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    __int128 out = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(v, out);
}

TEST(Int128, Negative)
{
    __int128 v = -(__int128(1) << 64);
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    __int128 out = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(v, out);
}

TEST(Uint128, Basic)
{
    unsigned __int128 v = ~(unsigned __int128)0;  // all bits set
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    unsigned __int128 out = 0;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(v, out);
}
#endif
