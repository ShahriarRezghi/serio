#include "common.h"

// ---- Serio::StaticArrayView ----

TEST(BinaryStaticArrayView, IntSerialize)
{
    int arr[5] = {1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<int, 5>(arr));
    int out[5] = {};
    auto view = Serio::StaticArrayView<int, 5>(out);
    Serio::deserialize<Serio::Binary>({}, bytes, view);
    for (int i = 0; i < 5; ++i) EXPECT_EQ(out[i], arr[i]);
}

TEST(BinaryStaticArrayView, DoubleRoundtrip)
{
    double arr[4] = {1.1, 2.2, 3.3, 4.4};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<double, 4>(arr));
    double out[4] = {};
    auto view = Serio::StaticArrayView<double, 4>(out);
    Serio::deserialize<Serio::Binary>({}, bytes, view);
    for (int i = 0; i < 4; ++i) EXPECT_DOUBLE_EQ(out[i], arr[i]);
}

TEST(BinaryStaticArrayView, Size1)
{
    int arr[1] = {99};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<int, 1>(arr));
    int out[1] = {0};
    auto view = Serio::StaticArrayView<int, 1>(out);
    Serio::deserialize<Serio::Binary>({}, bytes, view);
    EXPECT_EQ(out[0], 99);
}

TEST(BinaryStaticArrayView, ByteArray)
{
    uint8_t arr[8] = {0, 1, 127, 128, 200, 255, 42, 13};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<uint8_t, 8>(arr));
    uint8_t out[8] = {};
    auto view = Serio::StaticArrayView<uint8_t, 8>(out);
    Serio::deserialize<Serio::Binary>({}, bytes, view);
    for (int i = 0; i < 8; ++i) EXPECT_EQ(out[i], arr[i]);
}

TEST(BinaryStaticArrayView, NoLengthPrefix)
{
    // StaticArrayView has no length prefix, same as std::array
    int arr[3] = {1, 2, 3};
    std::array<int, 3> stdArr{1, 2, 3};
    auto b1 = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<int, 3>(arr));
    auto b2 = Serio::serialize<Serio::Binary>({}, stdArr);
    EXPECT_EQ(b1, b2);
}

TEST(BinaryStaticArrayView, InterchangeableWithArray)
{
    // Serialize as StaticArrayView, deserialize as std::array
    int arr[4] = {10, 20, 30, 40};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<int, 4>(arr));
    std::array<int, 4> out{};
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    for (int i = 0; i < 4; ++i) EXPECT_EQ(out[i], arr[i]);
}

TEST(BinaryStaticArrayView, DeserializeIntoRawBuffer)
{
    // Serialize as std::array, deserialize into StaticArrayView
    std::array<int, 3> arr{7, 8, 9};
    auto bytes = Serio::serialize<Serio::Binary>({}, arr);
    int out[3] = {};
    auto view = Serio::StaticArrayView<int, 3>(out);
    Serio::deserialize<Serio::Binary>({}, bytes, view);
    for (int i = 0; i < 3; ++i) EXPECT_EQ(out[i], arr[i]);
}

TEST(BinaryStaticArrayView, LargeArray)
{
    constexpr int N = 1000;
    std::vector<float> src(N);
    for (int i = 0; i < N; ++i) src[i] = static_cast<float>(i) * 0.1f;
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::StaticArrayView<float, N>(src.data()));
    std::vector<float> dst(N, 0.0f);
    auto view = Serio::StaticArrayView<float, N>(dst.data());
    Serio::deserialize<Serio::Binary>({}, bytes, view);
    for (int i = 0; i < N; ++i) EXPECT_EQ(dst[i], src[i]);
}

// ---- Serio::PointerView (serialize-only) ----

TEST(BinaryPointerView, IntBuffer)
{
    std::vector<int> src{1, 2, 3, 4, 5};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<int>(src.data(), src.size()));
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, src);
}

TEST(BinaryPointerView, EmptyBuffer)
{
    std::vector<int> src;
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<int>(src.data(), src.size()));
    std::vector<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}

TEST(BinaryPointerView, FloatBuffer)
{
    float arr[3] = {1.1f, 2.2f, 3.3f};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<float>(arr, 3));
    std::vector<float> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], 1.1f);
    EXPECT_EQ(out[1], 2.2f);
    EXPECT_EQ(out[2], 3.3f);
}

TEST(BinaryPointerView, DeserializeAsDeque)
{
    std::vector<int> src{10, 20, 30};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<int>(src.data(), src.size()));
    std::deque<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], 10);
    EXPECT_EQ(out[1], 20);
    EXPECT_EQ(out[2], 30);
}

TEST(BinaryPointerView, DeserializeAsSet)
{
    int arr[] = {3, 1, 2, 1, 3};
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<int>(arr, 5));
    std::set<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    std::set<int> expected{1, 2, 3};
    EXPECT_EQ(out, expected);
}

TEST(BinaryPointerView, LargeBuffer)
{
    std::vector<double> src(5000);
    for (std::size_t i = 0; i < src.size(); ++i) src[i] = static_cast<double>(i) * 0.001;
    auto bytes = Serio::serialize<Serio::Binary>({}, Serio::PointerView<double>(src.data(), src.size()));
    std::vector<double> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, src);
}

TEST(BinaryPointerView, SameAsVector)
{
    std::vector<int> src{7, 8, 9};
    auto b1 = Serio::serialize<Serio::Binary>({}, Serio::PointerView<int>(src.data(), src.size()));
    auto b2 = Serio::serialize<Serio::Binary>({}, src);
    EXPECT_EQ(b1, b2);
}
