#include "common.h"

// ---- std::shared_ptr ----

TEST(BinarySharedPtr, NullInt)
{
    std::shared_ptr<int> p;
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<int> out = std::make_shared<int>(99);
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, nullptr);
}

TEST(BinarySharedPtr, WithInt)
{
    auto p = std::make_shared<int>(42);
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 42);
}

TEST(BinarySharedPtr, WithString)
{
    auto p = std::make_shared<std::string>("hello");
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, "hello");
}

TEST(BinarySharedPtr, WithCustomType)
{
    auto p = std::make_shared<Point2D>(Point2D{1.0f, 2.0f});
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<Point2D> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, (Point2D{1.0f, 2.0f}));
}

TEST(BinarySharedPtr, WithVector)
{
    auto p = std::make_shared<std::vector<int>>(std::vector<int>{1, 2, 3, 4, 5});
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<std::vector<int>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(BinarySharedPtr, NullVsSet)
{
    // null pointer vs set pointer produce different bytes
    std::shared_ptr<int> null_p;
    auto p = std::make_shared<int>(0);
    auto b1 = Serio::serialize<Serio::Binary>({}, null_p);
    auto b2 = Serio::serialize<Serio::Binary>({}, p);
    EXPECT_NE(b1, b2);
}

TEST(BinarySharedPtr, ZeroValue)
{
    auto p = std::make_shared<int>(0);
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 0);
}

TEST(BinarySharedPtr, NullBool)
{
    std::shared_ptr<bool> p;
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<bool> out = std::make_shared<bool>(true);
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, nullptr);
}

// ---- std::unique_ptr ----

TEST(BinaryUniquePtr, NullInt)
{
    std::unique_ptr<int> p;
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::unique_ptr<int> out = std::make_unique<int>(99);
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, nullptr);
}

TEST(BinaryUniquePtr, WithInt)
{
    auto p = std::make_unique<int>(42);
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::unique_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 42);
}

TEST(BinaryUniquePtr, WithString)
{
    auto p = std::make_unique<std::string>("world");
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::unique_ptr<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, "world");
}

TEST(BinaryUniquePtr, WithCustomType)
{
    auto p = std::make_unique<Point3D>(Point3D{1.0, 2.0, 3.0});
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::unique_ptr<Point3D> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, (Point3D{1.0, 2.0, 3.0}));
}

TEST(BinaryUniquePtr, VectorOfUniquePtr)
{
    std::vector<std::unique_ptr<int>> v;
    v.push_back(std::make_unique<int>(1));
    v.push_back(nullptr);
    v.push_back(std::make_unique<int>(3));

    auto bytes = Serio::serialize<Serio::Binary>({}, v);
    std::vector<std::unique_ptr<int>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_EQ(out.size(), 3u);
    ASSERT_NE(out[0], nullptr);
    EXPECT_EQ(*out[0], 1);
    EXPECT_EQ(out[1], nullptr);
    ASSERT_NE(out[2], nullptr);
    EXPECT_EQ(*out[2], 3);
}

// ---- std::weak_ptr (serialize-only) ----

TEST(BinaryWeakPtr, SerializeAndDeserializeAsShared)
{
    auto shared = std::make_shared<int>(55);
    std::weak_ptr<int> wp = shared;
    auto bytes = Serio::serialize<Serio::Binary>({}, wp);
    std::shared_ptr<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    EXPECT_EQ(*out, 55);
}

TEST(BinaryWeakPtr, NullWeakPtr)
{
    // expired/empty weak_ptr should serialize as null presence flag
    std::weak_ptr<int> wp;
    auto bytes = Serio::serialize<Serio::Binary>({}, wp);
    std::shared_ptr<int> out = std::make_shared<int>(99);
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, nullptr);
}

TEST(BinaryWeakPtr, ExpiredWeakPtr)
{
    std::weak_ptr<int> wp;
    {
        auto shared = std::make_shared<int>(42);
        wp = shared;
    }
    // shared_ptr destroyed — weak_ptr is expired, should serialize as null
    auto bytes = Serio::serialize<Serio::Binary>({}, wp);
    std::shared_ptr<int> out = std::make_shared<int>(99);
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(out, nullptr);
}

// ---- Shared ptr of shared ptr ----

TEST(BinarySharedPtr, NestedSharedPtr)
{
    auto inner = std::make_shared<int>(77);
    auto p = std::make_shared<std::shared_ptr<int>>(inner);
    auto bytes = Serio::serialize<Serio::Binary>({}, p);
    std::shared_ptr<std::shared_ptr<int>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    ASSERT_NE(out, nullptr);
    ASSERT_NE(*out, nullptr);
    EXPECT_EQ(**out, 77);
}
