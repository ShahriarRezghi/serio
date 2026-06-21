#include <atomic>
#include <bitset>
#include <chrono>
#include <complex>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "common.h"

// ---- std::pair ----

TEST(Pair, IntInt) { check(std::pair<int, int>{1, 2}); }
TEST(Pair, IntString) { check(std::pair<int, std::string>{42, "hello"}); }
TEST(Pair, DoubleBool) { check(std::pair<double, bool>{3.14, true}); }
TEST(Pair, Empty) { check(std::pair<int, int>{0, 0}); }
TEST(Pair, Nested) { check(std::pair<std::pair<int, int>, std::string>{{1, 2}, "hi"}); }
TEST(Pair, VectorPair)
{
    std::vector<std::pair<int, std::string>> v{{1, "a"}, {2, "b"}, {3, "c"}};
    check(v);
}

// ---- std::tuple ----

TEST(Tuple, Single) { check(std::tuple<int>{42}); }
TEST(Tuple, Two) { check(std::tuple<int, std::string>{1, "hello"}); }
TEST(Tuple, Three) { check(std::tuple<bool, int, double>{true, 7, 3.14}); }
TEST(Tuple, Four) { check(std::tuple<int, int, int, int>{1, 2, 3, 4}); }
TEST(Tuple, Mixed)
{
    auto t = std::make_tuple(std::string{"world"}, 42, false, 1.5);
    check(t);
}
TEST(Tuple, Nested)
{
    auto t = std::make_tuple(std::tuple<int, int>{1, 2}, std::string{"ok"});
    check(t);
}

// ---- std::complex ----

TEST(Complex, FloatZero) { check(std::complex<float>{0.0f, 0.0f}); }
TEST(Complex, FloatBasic) { check(std::complex<float>{1.0f, 2.0f}); }
TEST(Complex, FloatNeg) { check(std::complex<float>{-3.0f, 4.0f}); }
TEST(Complex, DoubleZero) { check(std::complex<double>{0.0, 0.0}); }
TEST(Complex, DoubleBasic) { check(std::complex<double>{1.0, -1.0}); }
TEST(Complex, IntBased) { check(std::complex<int>{5, -5}); }
TEST(Complex, VectorOf)
{
    std::vector<std::complex<double>> v{{1.0, 0.0}, {0.0, 1.0}, {-1.0, -1.0}};
    check(v);
}

// ---- std::bitset<N> (bit-packed) ----

TEST(Bitset, N8_Zero) { check(std::bitset<8>{}); }
TEST(Bitset, N8_All) { check(std::bitset<8>{0xFF}); }
TEST(Bitset, N8_Alternating) { check(std::bitset<8>{0xAA}); }
TEST(Bitset, N1_False) { check(std::bitset<1>{}); }
TEST(Bitset, N1_True) { check(std::bitset<1>{1}); }
TEST(Bitset, N64_Zero) { check(std::bitset<64>{}); }
TEST(Bitset, N64_All) { check(std::bitset<64>(~0ULL)); }
TEST(Bitset, N100_Mixed)
{
    std::bitset<100> bs;
    for (size_t i = 0; i < 100; i += 3) bs.set(i);
    check(bs);
}

// ---- std::chrono::duration ----

TEST(ChronoDuration, Zero)
{
    auto d = std::chrono::nanoseconds{0};
    check(d);
}
TEST(ChronoDuration, Positive)
{
    auto d = std::chrono::milliseconds{12345};
    check(d);
}
TEST(ChronoDuration, Negative)
{
    auto d = std::chrono::microseconds{-500};
    check(d);
}
TEST(ChronoDuration, Seconds)
{
    auto d = std::chrono::seconds{3600};
    check(d);
}

// ---- std::chrono::time_point ----

TEST(ChronoTimePoint, Epoch)
{
    auto tp = std::chrono::system_clock::time_point{};
    check(tp);
}
TEST(ChronoTimePoint, Now)
{
    auto tp = std::chrono::system_clock::now();
    check(tp);
}
TEST(ChronoTimePoint, SteadyClock)
{
    auto tp = std::chrono::steady_clock::time_point{};
    check(tp);
}

// ---- std::atomic<T> ----

TEST(Atomic, IntZero)
{
    std::atomic<int> a{0};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<int> b{-1};
    Serio::deserialize<Serio::Binary>({}, bytes, b);
    EXPECT_EQ(a.load(), b.load());
}

TEST(Atomic, IntValue)
{
    std::atomic<int> a{12345};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<int> b{0};
    Serio::deserialize<Serio::Binary>({}, bytes, b);
    EXPECT_EQ(12345, b.load());
}

TEST(Atomic, Bool)
{
    std::atomic<bool> a{true};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<bool> b{false};
    Serio::deserialize<Serio::Binary>({}, bytes, b);
    EXPECT_EQ(true, b.load());
}

TEST(Atomic, UInt64)
{
    std::atomic<uint64_t> a{~0ULL};
    auto bytes = Serio::serialize<Serio::Binary>({}, a);
    std::atomic<uint64_t> b{0};
    Serio::deserialize<Serio::Binary>({}, bytes, b);
    EXPECT_EQ(a.load(), b.load());
}

// ---- std::shared_ptr<T> ----

TEST(SharedPtr, Null)
{
    std::shared_ptr<int> p;
    check(p);
}

// shared_ptr::operator== compares managed pointers, not pointed-to values.
// After deserialization a new allocation is made, so we dereference to compare.
template <typename T>
void check_sptr(const std::shared_ptr<T>& orig)
{
    auto check_one = [&](const std::shared_ptr<T>& rt) {
        ASSERT_EQ(bool(orig), bool(rt));
        if (orig) EXPECT_EQ(*orig, *rt);
    };
    check_one(mem_rt<Serio::Binary>(orig));
    check_one(mem_rt<Serio::JSON>(orig));
    check_one(mem_rt<Serio::XML>(orig));
    check_one(stream_rt<Serio::Binary>(orig));
    check_one(stream_rt<Serio::JSON>(orig));
    check_one(stream_rt<Serio::XML>(orig));
    check_one(file_rt<Serio::Binary>(orig));
    check_one(file_rt<Serio::JSON>(orig));
    check_one(file_rt<Serio::XML>(orig));
}

TEST(SharedPtr, NonNull) { check_sptr(std::make_shared<int>(42)); }

TEST(SharedPtr, String) { check_sptr(std::make_shared<std::string>("hello")); }

TEST(SharedPtr, Nested)
{
    // shared_ptr<shared_ptr<int>> — round-trip both layers
    auto inner = std::make_shared<int>(99);
    auto outer = std::make_shared<std::shared_ptr<int>>(inner);
    auto rt = mem_rt<Serio::Binary>(outer);
    ASSERT_TRUE(rt && *rt);
    EXPECT_EQ(**outer, **rt);
}

TEST(SharedPtr, Vector)
{
    std::vector<std::shared_ptr<int>> v;
    v.push_back(std::make_shared<int>(1));
    v.push_back(nullptr);
    v.push_back(std::make_shared<int>(3));

    auto check_vec = [&](const std::vector<std::shared_ptr<int>>& rt) {
        ASSERT_EQ(v.size(), rt.size());
        for (size_t i = 0; i < v.size(); ++i)
        {
            ASSERT_EQ(bool(v[i]), bool(rt[i]));
            if (v[i]) EXPECT_EQ(*v[i], *rt[i]);
        }
    };
    check_vec(mem_rt<Serio::Binary>(v));
    check_vec(mem_rt<Serio::JSON>(v));
    check_vec(mem_rt<Serio::XML>(v));
}

// ---- std::unique_ptr<T> ----

TEST(UniquePtr, Null)
{
    std::unique_ptr<int> p;
    // unique_ptr check: serialize then deserialize
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::unique_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(bool(p), bool(out));
}

TEST(UniquePtr, NonNull)
{
    auto p = std::make_unique<int>(77);
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::unique_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(bool(out));
    EXPECT_EQ(77, *out);
}

TEST(UniquePtr, String)
{
    auto p = std::make_unique<std::string>("unique string");
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::unique_ptr<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(bool(out));
    EXPECT_EQ("unique string", *out);
}

// ---- std::weak_ptr<T> (serialize-only — deserialize into shared_ptr) ----

TEST(WeakPtr, SerializeOnly)
{
    auto sp = std::make_shared<int>(55);
    std::weak_ptr<int> wp = sp;

    auto bytes = Serio::serialize<Serio::Binary>({}, wp);
    std::shared_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(bool(out));
    EXPECT_EQ(55, *out);
}

TEST(WeakPtr, Expired_SerializeOnly)
{
    std::weak_ptr<int> wp;
    {
        auto sp = std::make_shared<int>(0);
        wp = sp;
    }
    // wp is expired; it should serialize as null (no pointee)
    auto bytes = Serio::serialize<Serio::Binary>({}, wp);
    std::shared_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_FALSE(bool(out));
}

// ---- std::optional<T> ----

TEST(Optional, Empty_Int) { check(std::optional<int>{}); }
TEST(Optional, HasValue_Int) { check(std::optional<int>{42}); }
TEST(Optional, HasValue_Zero) { check(std::optional<int>{0}); }
TEST(Optional, String_Empty) { check(std::optional<std::string>{}); }
TEST(Optional, String_Value) { check(std::optional<std::string>{"hello"}); }
TEST(Optional, Nested) { check(std::optional<std::optional<int>>{std::optional<int>{7}}); }
TEST(Optional, VectorOf)
{
    std::vector<std::optional<int>> v{std::nullopt, 1, std::nullopt, 3};
    check(v);
}

// ---- std::variant<Ts...> ----

using V3 = std::variant<int, double, std::string>;

TEST(Variant, FirstAlt)
{
    V3 v{42};
    check(v);
}

TEST(Variant, SecondAlt)
{
    V3 v{3.14};
    check(v);
}

TEST(Variant, ThirdAlt)
{
    V3 v{std::string{"hello"}};
    check(v);
}

TEST(Variant, Monostate)
{
    std::variant<std::monostate, int, std::string> v{std::monostate{}};
    check(v);
}

TEST(Variant, VectorOf)
{
    std::vector<V3> vec{V3{1}, V3{2.0}, V3{std::string{"x"}}};
    check(vec);
}

// ---- std::monostate ----

TEST(Monostate, Standalone)
{
    std::monostate m;
    auto bytes = Serio::serialize<Serio::Binary>({}, m);
    std::monostate out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(m, out);
}

// ---- std::expected<T, E> (C++23) ----

TEST(Expected, Success_Int)
{
    std::expected<int, std::string> v{42};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::expected<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(42, out.value());
}

TEST(Expected, Error_String)
{
    std::expected<int, std::string> v{std::unexpected(std::string("something failed"))};
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::expected<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_FALSE(out.has_value());
    EXPECT_EQ("something failed", out.error());
}

TEST(Expected, Default_Success)
{
    std::expected<int, std::string> v{};  // default: success with int{}
    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::expected<int, std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(0, out.value());
}

TEST(Expected, JSON_Success)
{
    std::expected<int, std::string> v{100};
    auto bytes = Serio::serialize<Serio::JSON>({}, v);
    std::expected<int, std::string> out;
    Serio::deserialize<Serio::JSON>({}, bytes, out);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(100, out.value());
}

TEST(Expected, JSON_Error)
{
    std::expected<int, std::string> v{std::unexpected(std::string("err"))};
    auto bytes = Serio::serialize<Serio::JSON>({}, v);
    std::expected<int, std::string> out;
    Serio::deserialize<Serio::JSON>({}, bytes, out);
    ASSERT_FALSE(out.has_value());
    EXPECT_EQ("err", out.error());
}
