#include "common.h"

// ---- std::complex ----

TEST(BinaryComplex, Float)
{
    std::complex<float> c{3.0f, -4.0f};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(BinaryComplex, Double)
{
    std::complex<double> c{1.5, 2.5};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(BinaryComplex, Zero)
{
    std::complex<double> c{0.0, 0.0};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(BinaryComplex, PureImaginary)
{
    std::complex<double> c{0.0, 1.0};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(BinaryComplex, PureReal)
{
    std::complex<float> c{42.0f, 0.0f};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(BinaryComplex, Negative)
{
    std::complex<double> c{-1.0, -2.0};
    EXPECT_EQ(roundtrip_binary(c), c);
}

TEST(BinaryComplex, VectorOfComplex)
{
    std::vector<std::complex<float>> v{{1.0f, 0.0f}, {0.0f, 1.0f}, {3.0f, -4.0f}};
    EXPECT_EQ(roundtrip_binary(v), v);
}

// ---- std::bitset ----

TEST(BinaryBitset, Size8AllZero)
{
    std::bitset<8> b;
    EXPECT_EQ(roundtrip_binary(b), b);
}

TEST(BinaryBitset, Size8AllOne)
{
    std::bitset<8> b;
    b.set();
    EXPECT_EQ(roundtrip_binary(b), b);
}

TEST(BinaryBitset, Size8Mixed)
{
    std::bitset<8> b(0b10110011);
    EXPECT_EQ(roundtrip_binary(b), b);
}

TEST(BinaryBitset, Size1)
{
    std::bitset<1> b(1);
    EXPECT_EQ(roundtrip_binary(b), b);
}

TEST(BinaryBitset, Size64)
{
    std::bitset<64> b;
    b[0] = 1;
    b[31] = 1;
    b[63] = 1;
    EXPECT_EQ(roundtrip_binary(b), b);
}

TEST(BinaryBitset, Size100)
{
    std::bitset<100> b;
    for (int i = 0; i < 100; i += 3) b.set(i);
    EXPECT_EQ(roundtrip_binary(b), b);
}

TEST(BinaryBitset, Size1024)
{
    std::bitset<1024> b;
    for (int i = 0; i < 1024; i += 7) b.set(i);
    EXPECT_EQ(roundtrip_binary(b), b);
}

// ---- std::chrono::duration ----

TEST(BinaryChronoDuration, Nanoseconds)
{
    auto d = std::chrono::nanoseconds(123456789LL);
    EXPECT_EQ(roundtrip_binary(d), d);
}

TEST(BinaryChronoDuration, Microseconds)
{
    auto d = std::chrono::microseconds(9999999LL);
    EXPECT_EQ(roundtrip_binary(d), d);
}

TEST(BinaryChronoDuration, Milliseconds)
{
    auto d = std::chrono::milliseconds(5000LL);
    EXPECT_EQ(roundtrip_binary(d), d);
}

TEST(BinaryChronoDuration, Seconds)
{
    auto d = std::chrono::seconds(3661LL);
    EXPECT_EQ(roundtrip_binary(d), d);
}

TEST(BinaryChronoDuration, Minutes)
{
    auto d = std::chrono::minutes(90);
    EXPECT_EQ(roundtrip_binary(d), d);
}

TEST(BinaryChronoDuration, Hours)
{
    auto d = std::chrono::hours(24);
    EXPECT_EQ(roundtrip_binary(d), d);
}

TEST(BinaryChronoDuration, Zero)
{
    auto d = std::chrono::seconds(0);
    EXPECT_EQ(roundtrip_binary(d), d);
}

TEST(BinaryChronoDuration, Negative)
{
    auto d = std::chrono::milliseconds(-500LL);
    EXPECT_EQ(roundtrip_binary(d), d);
}

// ---- std::chrono::time_point ----

TEST(BinaryChronoTimepoint, Epoch)
{
    std::chrono::system_clock::time_point tp{};
    EXPECT_EQ(roundtrip_binary(tp), tp);
}

TEST(BinaryChronoTimepoint, Now)
{
    auto tp = std::chrono::system_clock::now();
    EXPECT_EQ(roundtrip_binary(tp), tp);
}

TEST(BinaryChronoTimepoint, SteadyClock)
{
    auto tp = std::chrono::steady_clock::now();
    EXPECT_EQ(roundtrip_binary(tp), tp);
}

TEST(BinaryChronoTimepoint, FromDuration)
{
    using namespace std::chrono;
    system_clock::time_point tp(hours(24 * 365 * 50));  // ~50 years after epoch
    EXPECT_EQ(roundtrip_binary(tp), tp);
}

// ---- std::atomic ----

TEST(BinaryAtomic, Int)
{
    std::atomic<int> a{42};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<int> out{0};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.load(), 42);
}

TEST(BinaryAtomic, Bool)
{
    std::atomic<bool> a{true};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<bool> out{false};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.load(), true);
}

TEST(BinaryAtomic, Uint64)
{
    std::atomic<uint64_t> a{std::numeric_limits<uint64_t>::max()};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<uint64_t> out{0};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.load(), std::numeric_limits<uint64_t>::max());
}

TEST(BinaryAtomic, Zero)
{
    std::atomic<int> a{0};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<int> out{99};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out.load(), 0);
}
