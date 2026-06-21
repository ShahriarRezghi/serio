#include "common.h"

// Helper: drain queue/stack into vector for comparison
template <typename T>
std::vector<T> drain_queue(std::queue<T> q)
{
    std::vector<T> v;
    while (!q.empty())
    {
        v.push_back(q.front());
        q.pop();
    }
    return v;
}

template <typename T>
std::vector<T> drain_stack(std::stack<T> s)
{
    std::vector<T> v;
    while (!s.empty())
    {
        v.push_back(s.top());
        s.pop();
    }
    return v;
}

template <typename T, typename C = std::vector<T>, typename Cmp = std::less<T>>
std::vector<T> drain_pq(std::priority_queue<T, C, Cmp> pq)
{
    std::vector<T> v;
    while (!pq.empty())
    {
        v.push_back(pq.top());
        pq.pop();
    }
    return v;
}

// ---- std::queue ----

TEST(BinaryQueue, Empty)
{
    std::queue<int> q;
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_queue(q), drain_queue(out));
}

TEST(BinaryQueue, Ints)
{
    std::queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_queue(q), drain_queue(out));
}

TEST(BinaryQueue, Strings)
{
    std::queue<std::string> q;
    q.push("hello");
    q.push("world");
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_queue(q), drain_queue(out));
}

TEST(BinaryQueue, Large)
{
    std::queue<int> q;
    for (int i = 0; i < 1000; ++i) q.push(i);
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_queue(q), drain_queue(out));
}

// ---- std::stack ----

TEST(BinaryStack, Empty)
{
    std::stack<int> s;
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_stack(s), drain_stack(out));
}

TEST(BinaryStack, Ints)
{
    std::stack<int> s;
    s.push(10);
    s.push(20);
    s.push(30);
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    // stack serializes from bottom to top, order should be preserved
    EXPECT_EQ(drain_stack(s), drain_stack(out));
}

TEST(BinaryStack, Strings)
{
    std::stack<std::string> s;
    s.push("a");
    s.push("b");
    s.push("c");
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_stack(s), drain_stack(out));
}

TEST(BinaryStack, CustomType)
{
    std::stack<Point2D> s;
    s.push({1.0f, 2.0f});
    s.push({3.0f, 4.0f});
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<Point2D> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    auto v1 = drain_stack(s), v2 = drain_stack(out);
    EXPECT_EQ(v1, v2);
}

// ---- std::priority_queue ----

TEST(BinaryPriorityQueue, Empty)
{
    std::priority_queue<int> pq;
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_pq(pq), drain_pq(out));
}

TEST(BinaryPriorityQueue, Ints)
{
    std::priority_queue<int> pq;
    pq.push(5);
    pq.push(1);
    pq.push(3);
    pq.push(2);
    pq.push(4);
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_pq(pq), drain_pq(out));
}

TEST(BinaryPriorityQueue, MinHeap)
{
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    pq.push(5);
    pq.push(1);
    pq.push(3);
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int, std::vector<int>, std::greater<int>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_pq(pq), drain_pq(out));
}

TEST(BinaryPriorityQueue, Strings)
{
    std::priority_queue<std::string> pq;
    pq.push("banana");
    pq.push("apple");
    pq.push("cherry");
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_pq(pq), drain_pq(out));
}

TEST(BinaryPriorityQueue, Large)
{
    std::priority_queue<int> pq;
    for (int i = 0; i < 500; ++i) pq.push(i * 13 % 97);
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_EQ(drain_pq(pq), drain_pq(out));
}
