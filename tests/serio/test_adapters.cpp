#include <queue>
#include <stack>
#include <string>

#include "common.h"

// ---- std::queue<T> ----

TEST(Queue, Empty_Int)
{
    std::queue<int> q;
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}

TEST(Queue, Single_Int)
{
    std::queue<int> q;
    q.push(42);
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    compare_queue(q, out);
}

TEST(Queue, Multi_Int)
{
    std::queue<int> q;
    for (int i : {10, 20, 30, 40, 50}) q.push(i);
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    compare_queue(q, out);
}

TEST(Queue, String)
{
    std::queue<std::string> q;
    q.push("hello");
    q.push("world");
    q.push("");
    auto bytes = Serio::serialize<Serio::Binary>({}, q);
    std::queue<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    compare_queue(q, out);
}

TEST(Queue, JSON)
{
    std::queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    auto bytes = Serio::serialize<Serio::JSON>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::JSON>({}, bytes, out);
    compare_queue(q, out);
}

TEST(Queue, XML)
{
    std::queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    auto bytes = Serio::serialize<Serio::XML>({}, q);
    std::queue<int> out;
    Serio::deserialize<Serio::XML>({}, bytes, out);
    compare_queue(q, out);
}

// ---- std::stack<T> ----

TEST(Stack, Empty_Int)
{
    std::stack<int> s;
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}

TEST(Stack, Single_Int)
{
    std::stack<int> s;
    s.push(99);
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    compare_stack(s, out);
}

TEST(Stack, Multi_Int)
{
    std::stack<int> s;
    for (int i : {1, 2, 3, 4, 5}) s.push(i);
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    compare_stack(s, out);
}

TEST(Stack, String)
{
    std::stack<std::string> s;
    s.push("a");
    s.push("bb");
    s.push("ccc");
    auto bytes = Serio::serialize<Serio::Binary>({}, s);
    std::stack<std::string> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    compare_stack(s, out);
}

TEST(Stack, JSON)
{
    std::stack<int> s;
    s.push(10);
    s.push(20);
    s.push(30);
    auto bytes = Serio::serialize<Serio::JSON>({}, s);
    std::stack<int> out;
    Serio::deserialize<Serio::JSON>({}, bytes, out);
    compare_stack(s, out);
}

TEST(Stack, XML)
{
    std::stack<int> s;
    s.push(10);
    s.push(20);
    s.push(30);
    auto bytes = Serio::serialize<Serio::XML>({}, s);
    std::stack<int> out;
    Serio::deserialize<Serio::XML>({}, bytes, out);
    compare_stack(s, out);
}

// ---- std::priority_queue<T> ----

TEST(PriorityQueue, Empty_Int)
{
    std::priority_queue<int> pq;
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    EXPECT_TRUE(out.empty());
}

TEST(PriorityQueue, Single_Int)
{
    std::priority_queue<int> pq;
    pq.push(42);
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    check_pq(pq, out);
}

TEST(PriorityQueue, Multi_Int)
{
    std::priority_queue<int> pq;
    for (int v : {3, 1, 4, 1, 5, 9, 2, 6}) pq.push(v);
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    check_pq(pq, out);
}

TEST(PriorityQueue, MinHeap)
{
    // Min-heap using greater<int>
    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    for (int v : {3, 1, 4, 1, 5}) pq.push(v);
    auto bytes = Serio::serialize<Serio::Binary>({}, pq);
    std::priority_queue<int, std::vector<int>, std::greater<int>> out;
    Serio::deserialize<Serio::Binary>({}, bytes, out);
    check_pq(pq, out);
}

TEST(PriorityQueue, JSON)
{
    std::priority_queue<int> pq;
    pq.push(5);
    pq.push(3);
    pq.push(8);
    auto bytes = Serio::serialize<Serio::JSON>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::JSON>({}, bytes, out);
    check_pq(pq, out);
}

TEST(PriorityQueue, XML)
{
    std::priority_queue<int> pq;
    pq.push(5);
    pq.push(3);
    pq.push(8);
    auto bytes = Serio::serialize<Serio::XML>({}, pq);
    std::priority_queue<int> out;
    Serio::deserialize<Serio::XML>({}, bytes, out);
    check_pq(pq, out);
}
