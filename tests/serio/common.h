#pragma once

#include <gtest/gtest.h>
#include <serio/serio.h>

#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <deque>
#include <expected>
#include <filesystem>
#include <forward_list>
#include <fstream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <span>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <valarray>
#include <variant>
#include <vector>

static const char* TMP_FILE = "new_serio_test.tmp";

// ---- Low-level round-trip helpers ----

template <Serio::Type ST, typename T>
T mem_rt(const T& v, Serio::SerializeOptions so = {}, Serio::DeserializeOptions dopt = {})
{
    T out{};
    auto buf = Serio::serialize<ST>(so, v);
    Serio::deserialize<ST>(dopt, buf, out);
    return out;
}

template <Serio::Type ST, typename T>
T stream_rt(const T& v)
{
    {
        std::ofstream os(TMP_FILE, std::ios::binary);
        Serio::write<ST>({}, os, v);
    }
    T out{};
    {
        std::ifstream is(TMP_FILE, std::ios::binary);
        Serio::read<ST>({}, is, out);
    }
    return out;
}

template <Serio::Type ST, typename T>
T file_rt(const T& v, Serio::SerializeOptions so = {}, Serio::DeserializeOptions dopt = {})
{
    Serio::save<ST>(so, TMP_FILE, v);
    T out{};
    Serio::load<ST>(dopt, TMP_FILE, out);
    return out;
}

// ---- High-level round-trip helpers ----

template <auto Format, typename T>
T roundtrip(const T& value, Serio::SerializeOptions sopt = {}, Serio::DeserializeOptions dopt = {})
{
    auto bytes = Serio::serialize<Format>(sopt, value);
    T out{};
    Serio::deserialize<Format>(dopt, bytes, out);
    return out;
}

template <typename T>
T roundtrip_binary(const T& value)
{
    return roundtrip<Serio::Binary>(value);
}

template <typename T>
T roundtrip_json(const T& value)
{
    return roundtrip<Serio::JSON>(value);
}

template <typename T>
T roundtrip_xml(const T& value)
{
    return roundtrip<Serio::XML>(value);
}

// ---- Check helpers (all backends × all I/O methods) ----

template <typename T>
void check(const T& v)
{
    EXPECT_EQ(v, mem_rt<Serio::Binary>(v));
    EXPECT_EQ(v, mem_rt<Serio::JSON>(v));
    EXPECT_EQ(v, mem_rt<Serio::XML>(v));
    EXPECT_EQ(v, stream_rt<Serio::Binary>(v));
    EXPECT_EQ(v, stream_rt<Serio::JSON>(v));
    EXPECT_EQ(v, stream_rt<Serio::XML>(v));
    EXPECT_EQ(v, file_rt<Serio::Binary>(v));
    EXPECT_EQ(v, file_rt<Serio::JSON>(v));
    EXPECT_EQ(v, file_rt<Serio::XML>(v));
}

template <typename T>
void check_bin(const T& v)
{
    EXPECT_EQ(v, mem_rt<Serio::Binary>(v));
    EXPECT_EQ(v, stream_rt<Serio::Binary>(v));
    EXPECT_EQ(v, file_rt<Serio::Binary>(v));
}

template <typename T>
void check_valarray(const std::valarray<T>& v)
{
    auto cmp = [](const std::valarray<T>& a, const std::valarray<T>& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) return false;
        return true;
    };

    EXPECT_TRUE(cmp(v, mem_rt<Serio::Binary>(v)));
    EXPECT_TRUE(cmp(v, mem_rt<Serio::JSON>(v)));
    EXPECT_TRUE(cmp(v, mem_rt<Serio::XML>(v)));
    EXPECT_TRUE(cmp(v, file_rt<Serio::Binary>(v)));
    EXPECT_TRUE(cmp(v, stream_rt<Serio::Binary>(v)));
}

template <typename T, typename Container = std::vector<T>, typename Cmp = std::less<T>>
void check_pq(std::priority_queue<T, Container, Cmp> q1, std::priority_queue<T, Container, Cmp> q2)
{
    EXPECT_EQ(q1.size(), q2.size());
    while (!q1.empty() && !q2.empty())
    {
        EXPECT_EQ(q1.top(), q2.top());
        q1.pop();
        q2.pop();
    }
}

template <typename T>
void compare_queue(std::queue<T> q1, std::queue<T> q2)
{
    EXPECT_EQ(q1.size(), q2.size());
    while (!q1.empty() && !q2.empty())
    {
        EXPECT_EQ(q1.front(), q2.front());
        q1.pop();
        q2.pop();
    }
}

template <typename T>
void compare_stack(std::stack<T> s1, std::stack<T> s2)
{
    EXPECT_EQ(s1.size(), s2.size());
    while (!s1.empty() && !s2.empty())
    {
        EXPECT_EQ(s1.top(), s2.top());
        s1.pop();
        s2.pop();
    }
}

// ---- Common registered types ----

struct Point2D
{
    float x{}, y{};
    bool operator==(const Point2D& o) const { return x == o.x && y == o.y; }
    SERIO_REGISTER(x, y)
};

struct Point3D
{
    double x{}, y{}, z{};
    bool operator==(const Point3D& o) const { return x == o.x && y == o.y && z == o.z; }
    SERIO_REGISTER(x, y, z)
};

struct Named
{
    std::string name;
    int value{};
    bool operator==(const Named& o) const { return name == o.name && value == o.value; }
    SERIO_REGISTER(name, value)
};

struct Nested
{
    Point2D point;
    std::vector<int> data;
    std::string label;
    bool operator==(const Nested& o) const { return point == o.point && data == o.data && label == o.label; }
    SERIO_REGISTER(point, data, label)
};

struct AllBuiltins
{
    bool b{};
    int8_t i8{};
    uint8_t u8{};
    int16_t i16{};
    uint16_t u16{};
    int32_t i32{};
    uint32_t u32{};
    int64_t i64{};
    uint64_t u64{};
    float f{};
    double d{};
    bool operator==(const AllBuiltins& o) const
    {
        return b == o.b && i8 == o.i8 && u8 == o.u8 && i16 == o.i16 && u16 == o.u16 && i32 == o.i32 && u32 == o.u32 &&
               i64 == o.i64 && u64 == o.u64 && f == o.f && d == o.d;
    }
    SERIO_REGISTER(b, i8, u8, i16, u16, i32, u32, i64, u64, f, d)
};

// External type adapted via Serio::CustomClass without modifying the original
struct Vec3
{
    float x{}, y{}, z{};
    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
};

namespace Serio
{
template <>
struct CustomClass<Vec3>
{
    template <typename S>
    void serialize(const Vec3& v, S& C)
    {
        C.process(v.x, v.y, v.z);
    }
    template <typename D>
    void deserialize(Vec3& v, D& C)
    {
        C.process(v.x, v.y, v.z);
    }
};
}  // namespace Serio

// Type with NVP-based serialization
struct Config
{
    bool enabled{};
    int retries{};
    std::string host;
    bool operator==(const Config& o) const { return enabled == o.enabled && retries == o.retries && host == o.host; }

    SERIO_FRIEND;

    template <typename S>
    void _serialize(S& C) const
    {
        C.process(Serio::nvp("enabled", enabled), Serio::nvp("retries", retries), Serio::nvp("host", host));
    }
    template <typename D>
    void _deserialize(D& C)
    {
        auto en = Serio::nvp("enabled", enabled);
        auto re = Serio::nvp("retries", retries);
        auto ho = Serio::nvp("host", host);
        C.process(en, re, ho);
    }
};

// Custom container with manual size-prefix serialization (binary-only)
class IntBag
{
public:
    std::vector<int> data;
    IntBag() = default;
    explicit IntBag(std::vector<int> d) : data(std::move(d)) {}
    bool operator==(const IntBag& o) const { return data == o.data; }

    SERIO_FRIEND;

    template <typename S>
    void _serialize(S& C) const
    {
        C << Serio::Size(data.size());
        for (const auto& v : data) C << v;
    }
    template <typename D>
    void _deserialize(D& C)
    {
        Serio::Size sz;
        C >> sz;
        data.resize(static_cast<std::size_t>(sz));
        for (auto& v : data) C >> v;
    }
};

inline std::string temp_path(const std::string& suffix = ".bin")
{
    return (std::filesystem::temp_directory_path() / ("serio_test_" + suffix)).string();
}
