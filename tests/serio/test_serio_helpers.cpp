#include <array>
#include <span>
#include <string>
#include <vector>

#include "common.h"

// Blob must be at file scope: SERIO_REGISTER expands to template members,
// which are not allowed in local classes (inside TEST() bodies).
struct Blob
{
    std::string data;
    bool operator==(const Blob& o) const { return data == o.data; }
    SERIO_REGISTER(Serio::binaryString(data))
};

// ---- Serio::PointerView<T> (serialize-only) ----

TEST(PointerView, IntBuffer)
{
    std::vector<int> data{1, 2, 3, 4, 5};
    Serio::PointerView<int> pv(data.data(), data.size());

    auto bytes = Serio::serialize<Serio::Binary>({}, pv);

    // PointerView writes a count prefix — read back as vector
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(data, out);
}

TEST(PointerView, Empty)
{
    std::vector<int> data;
    Serio::PointerView<int> pv(data.data(), 0);

    auto bytes = Serio::serialize<Serio::Binary>({}, pv);
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}

TEST(PointerView, FloatBuffer)
{
    float buf[] = {1.0f, 2.0f, 3.0f};
    Serio::PointerView<float> pv(buf, 3);

    auto bytes = Serio::serialize<Serio::Binary>({}, pv);
    std::vector<float> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(3u, out.size());
    EXPECT_EQ(1.0f, out[0]);
    EXPECT_EQ(2.0f, out[1]);
    EXPECT_EQ(3.0f, out[2]);
}

TEST(PointerView, IntoList)
{
    // PointerView writes count+elements, readable as any resizable sequence
    int buf[] = {10, 20, 30};
    Serio::PointerView<int> pv(buf, 3);

    auto bytes = Serio::serialize<Serio::Binary>({}, pv);
    std::list<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(std::list<int>({10, 20, 30}), out);
}

// ---- Serio::StaticArrayView<T, N> (both directions) ----

TEST(StaticArrayView, Serialize)
{
    int buf[4] = {1, 2, 3, 4};
    Serio::StaticArrayView<int, 4> sav(buf);

    auto bytes = Serio::serialize<Serio::Binary>({}, sav);

    int out_buf[4] = {};
    Serio::StaticArrayView<int, 4> sav_out(out_buf);
    Serio::deserialize<Serio::Binary>({}, bytes, sav_out);

    for (int i = 0; i < 4; ++i) EXPECT_EQ(buf[i], out_buf[i]);
}

TEST(StaticArrayView, DoubleBuffer)
{
    double buf[3] = {1.0, 2.0, 3.0};
    Serio::StaticArrayView<double, 3> sav(buf);

    auto bytes = Serio::serialize<Serio::Binary>({}, sav);

    double out_buf[3] = {};
    Serio::StaticArrayView<double, 3> sav_out(out_buf);
    Serio::deserialize<Serio::Binary>({}, bytes, sav_out);

    EXPECT_EQ(1.0, out_buf[0]);
    EXPECT_EQ(2.0, out_buf[1]);
    EXPECT_EQ(3.0, out_buf[2]);
}

TEST(StaticArrayView, NoLengthPrefix)
{
    // StaticArrayView<T,N> has no length prefix — interchangeable with array<T,N>
    int buf[3] = {7, 8, 9};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<int, 3>(buf));

    std::array<int, 3> arr{};
    Serio::deserialize<Serio::Binary>({}, bytes, arr);
    EXPECT_EQ(7, arr[0]);
    EXPECT_EQ(8, arr[1]);
    EXPECT_EQ(9, arr[2]);
}

TEST(StaticArrayView, FromArray)
{
    // Serialize std::array, deserialize into StaticArrayView
    std::array<int, 3> arr{100, 200, 300};
    auto bytes = Serio::serialize<Serio::Binary>({}, arr);

    int buf[3] = {};
    Serio::StaticArrayView<int, 3> sav(buf);
    Serio::deserialize<Serio::Binary>({}, bytes, sav);

    EXPECT_EQ(100, buf[0]);
    EXPECT_EQ(200, buf[1]);
    EXPECT_EQ(300, buf[2]);
}

// ---- Serio::binaryString() (base64 in JSON/XML) ----

// binaryString forces base64 encoding in JSON/XML so arbitrary bytes survive
TEST(BinaryString, ArbitraryBytesJSON)
{
    std::string original("\x00\x01\x7f\x80\xfe\xff", 6);
    Blob b1{original};
    auto json = Serio::serialize<Serio::JSON>({}, b1);
    Blob b2;
    Serio::deserialize<Serio::JSON>({}, json, b2);
    EXPECT_EQ(original, b2.data);
}

TEST(BinaryString, ArbitraryBytesXML)
{
    std::string original("\x00\x01\x7f\x80\xfe\xff", 6);
    Blob b1{original};
    auto xml = Serio::serialize<Serio::XML>({}, b1);
    Blob b2;
    Serio::deserialize<Serio::XML>({}, xml, b2);
    EXPECT_EQ(original, b2.data);
}

TEST(BinaryString, EmptyString)
{
    Blob b1{""};
    auto json = Serio::serialize<Serio::JSON>({}, b1);
    Blob b2;
    Serio::deserialize<Serio::JSON>({}, json, b2);
    EXPECT_EQ("", b2.data);
}

TEST(BinaryString, AsciiRoundTrip)
{
    Blob b1{"hello ASCII"};
    auto json = Serio::serialize<Serio::JSON>({}, b1);
    Blob b2;
    Serio::deserialize<Serio::JSON>({}, json, b2);
    EXPECT_EQ("hello ASCII", b2.data);
}

TEST(BinaryString, BinaryBackendNoOp)
{
    std::string original("\x00\xff\xfe", 3);
    Blob b1{original};
    auto bytes = Serio::serialize<Serio::Binary>({}, b1);
    Blob b2;
    Serio::deserialize<Serio::Binary>({}, bytes, b2);
    EXPECT_EQ(original, b2.data);
}

// ---- Serio::nvp() — named fields in JSON/XML ----
// Config with NVP serialization is defined in common.h

TEST(NVP, JSON_RoundTrip)
{
    Config c{true, 3, "localhost"};
    auto json = Serio::serialize<Serio::JSON>({}, c);
    Config out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    EXPECT_EQ(c, out);
}

TEST(NVP, XML_RoundTrip)
{
    Config c{false, 5, "example.com"};
    auto xml = Serio::serialize<Serio::XML>({}, c);
    Config out;
    Serio::deserialize<Serio::XML>({}, xml, out);
    EXPECT_EQ(c, out);
}

TEST(NVP, JSON_ContainsFieldNames)
{
    Config c{true, 3, "localhost"};
    auto json = Serio::serialize<Serio::JSON>({}, c);
    EXPECT_NE(std::string::npos, json.find("enabled"));
    EXPECT_NE(std::string::npos, json.find("retries"));
    EXPECT_NE(std::string::npos, json.find("host"));
}

TEST(NVP, VectorOfNVP_JSON)
{
    std::vector<Config> v{
        {true, 1, "a.com"},
        {false, 0, "b.org"},
    };
    auto json = Serio::serialize<Serio::JSON>({}, v);
    std::vector<Config> out;
    Serio::deserialize<Serio::JSON>({}, json, out);
    ASSERT_EQ(2u, out.size());
    EXPECT_EQ(v[0], out[0]);
    EXPECT_EQ(v[1], out[1]);
}

TEST(NVP, VectorOfNVP_XML)
{
    std::vector<Config> v{
        {true, 1, "a.com"},
        {false, 0, "b.org"},
    };
    auto xml = Serio::serialize<Serio::XML>({}, v);
    std::vector<Config> out;
    Serio::deserialize<Serio::XML>({}, xml, out);
    ASSERT_EQ(2u, out.size());
    EXPECT_EQ(v[0], out[0]);
    EXPECT_EQ(v[1], out[1]);
}
