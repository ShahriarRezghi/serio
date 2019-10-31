/*
  Copyright (c) 2019, Shahriar Rezghi
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Serio nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL SHAHRIAR REZGHI BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <float.h>
#include <gtest/gtest.h>
#include <math.h>
#include <serio.h>
#include <time.h>

#include <limits>
#include <random>

using namespace std;

#define CREATE_INIT(TYPE)                                                                   \
    void Init(TYPE& V)                                                                      \
    {                                                                                       \
        V = static_cast<TYPE>((rand() / double(RAND_MAX) - .5) * pow(2, sizeof(TYPE) * 8)); \
    }

#define CREATE_ITER_TEST_TYPE_0_DEPTH(NAME, TYPE, BNAME, BASE_TYPE) \
    TEST(NAME##_##BNAME, Serio) { TestIter(TYPE<BASE_TYPE>()); }

#define CREATE_ITER_TEST_TYPE(NAME, TYPE, BNAME, BASE_TYPE) \
    TEST(NAME##_##BNAME, Serio)                             \
    {                                                       \
        TestIter(TYPE<BASE_TYPE>());                        \
        TestIter(TYPE<TYPE<BASE_TYPE>>());                  \
    }

#ifdef __SIZEOF_INT128__
#define CREATE_ITER_TEST_BASE(MACRO, NAME, TYPE)               \
    MACRO(NAME, TYPE, char, char);                             \
    MACRO(NAME, TYPE, signed_char, signed char);               \
    MACRO(NAME, TYPE, short, short);                           \
    MACRO(NAME, TYPE, int, int);                               \
    MACRO(NAME, TYPE, long, long);                             \
    MACRO(NAME, TYPE, long_long, long long);                   \
    MACRO(NAME, TYPE, unsigned_char, unsigned char);           \
    MACRO(NAME, TYPE, unsigned_short, unsigned short);         \
    MACRO(NAME, TYPE, unsigned_int, unsigned int);             \
    MACRO(NAME, TYPE, unsigned_long, unsigned long);           \
    MACRO(NAME, TYPE, unsigned_long_long, unsigned long long); \
    MACRO(NAME, TYPE, wchar, wchar_t);                         \
    MACRO(NAME, TYPE, float, float);                           \
    MACRO(NAME, TYPE, double, double);                         \
    MACRO(NAME, TYPE, long_double, long double);
#else

#define CREATE_ITER_TEST_BASE(MACRO, NAME, TYPE)               \
    MACRO(NAME, TYPE, char, char);                             \
    MACRO(NAME, TYPE, signed_char, signed char);               \
    MACRO(NAME, TYPE, short, short);                           \
    MACRO(NAME, TYPE, int, int);                               \
    MACRO(NAME, TYPE, long, long);                             \
    MACRO(NAME, TYPE, long_long, long long);                   \
    MACRO(NAME, TYPE, unsigned_char, unsigned char);           \
    MACRO(NAME, TYPE, unsigned_short, unsigned short);         \
    MACRO(NAME, TYPE, unsigned_int, unsigned int);             \
    MACRO(NAME, TYPE, unsigned_long, unsigned long);           \
    MACRO(NAME, TYPE, unsigned_long_long, unsigned long long); \
    MACRO(NAME, TYPE, wchar, wchar_t);                         \
    MACRO(NAME, TYPE, float, float);                           \
    MACRO(NAME, TYPE, double, double);
#endif

#define CREATE_ITER_TEST(NAME, TYPE)               \
    CREATE_ITER_TEST_TYPE(NAME, TYPE, bool, bool); \
    CREATE_ITER_TEST_BASE(CREATE_ITER_TEST_TYPE, NAME, TYPE)

#define CREATE_ITER_TEST_0_DEPTH(NAME, TYPE)               \
    CREATE_ITER_TEST_TYPE_0_DEPTH(NAME, TYPE, bool, bool); \
    CREATE_ITER_TEST_BASE(CREATE_ITER_TEST_TYPE_0_DEPTH, NAME, TYPE)

#define CREATE_MAP_TEST_TYPE(NAME, TYPE, BNAME, BASE_TYPE)       \
    TEST(NAME##_##BNAME, Serio)                                  \
    {                                                            \
        TestIter(TYPE<BASE_TYPE, BASE_TYPE>());                  \
        TestIter(TYPE<BASE_TYPE, TYPE<BASE_TYPE, BASE_TYPE>>()); \
    }

#define CREATE_MAP_TEST(NAME, TYPE) CREATE_ITER_TEST_BASE(CREATE_MAP_TEST_TYPE, NAME, TYPE)

CREATE_INIT(char)
CREATE_INIT(signed char)
CREATE_INIT(short)
CREATE_INIT(int)
CREATE_INIT(long)
CREATE_INIT(long long)
CREATE_INIT(unsigned char)
CREATE_INIT(unsigned short)
CREATE_INIT(unsigned int)
CREATE_INIT(unsigned long)
CREATE_INIT(unsigned long long)
CREATE_INIT(wchar_t)
CREATE_INIT(bool)
CREATE_INIT(float)
CREATE_INIT(double)
#ifdef __SIZEOF_INT128__
CREATE_INIT(long double)
#endif

template <typename T>
inline void IteratableAdd(std::valarray<T>& I, T& value)
{
    I.resize(I.size() + 1);
    I[I.size() - 1] = value;
}
template <typename T, typename Alloc>
inline void IteratableAdd(std::vector<T, Alloc>& I, T& value)
{
    I.push_back(std::move(value));
}
template <typename T, typename Alloc>
inline void IteratableAdd(std::list<T, Alloc>& I, T& value)
{
    I.push_back(std::move(value));
}
template <typename T, typename Alloc>
inline void IteratableAdd(std::deque<T, Alloc>& I, T& value)
{
    I.push_back(std::move(value));
}
template <typename T, typename Alloc>
inline void IteratableAdd(std::forward_list<T, Alloc>& I, T& value)
{
    I.emplace_after(I.before_begin(), std::move(value));
}
template <typename T, typename Comp, typename Alloc>
inline void IteratableAdd(std::set<T, Comp, Alloc>& I, T& value)
{
    I.emplace(std::move(value));
}
template <typename T, typename Comp, typename Alloc>
inline void IteratableAdd(std::multiset<T, Comp, Alloc>& I, T& value)
{
    I.emplace(std::move(value));
}
template <typename T, typename Hash, typename Pred, typename Alloc>
inline void IteratableAdd(std::unordered_set<T, Hash, Pred, Alloc>& I, T& value)
{
    I.emplace(std::move(value));
}
template <typename T, typename Hash, typename Pred, typename Alloc>
inline void IteratableAdd(std::unordered_multiset<T, Hash, Pred, Alloc>& I, T& value)
{
    I.emplace(std::move(value));
}
template <typename K, typename T, typename Comp, typename Alloc>
inline void IteratableAdd(std::map<K, T, Comp, Alloc>& I, std::pair<K, T>& value)
{
    I.emplace(std::move(value));
}
template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
inline void IteratableAdd(std::unordered_map<K, T, Hash, Pred, Alloc>& I, std::pair<K, T>& value)
{
    I.emplace(std::move(value));
}
template <typename K, typename T, typename Comp, typename Alloc>
inline void IteratableAdd(std::multimap<K, T, Comp, Alloc>& I, std::pair<K, T>& value)
{
    I.emplace(std::move(value));
}
template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
inline void IteratableAdd(std::unordered_multimap<K, T, Hash, Pred, Alloc>& I,
                          std::pair<K, T>& value)
{
    I.emplace(std::move(value));
}
template <typename T, typename Traits, typename Alloc>
inline void IteratableAdd(std::basic_string<T, Traits, Alloc>& I, T& value)
{
    I.push_back(std::move(value));
}

template <typename T>
void Init(complex<T>& C);

template <typename Iter>
void Init(Iter& I)
{
    auto size = size_t(rand() % 1000);
    for (size_t i = 0; i < size; ++i)
    {
        auto V = typename Iter::value_type();
        Init(V);
        IteratableAdd(I, V);
    }
}

template <typename T>
void Init(complex<T>& C)
{
    T real, imag;
    Init(real);
    Init(imag);
    C = complex<T>(real, imag);
}

template <typename K, typename T>
void Init(pair<K, T>& pair);

template <typename K, typename T, typename Comp, typename Alloc>
void Init(std::map<K, T, Comp, Alloc>& I);

template <typename K, typename T, typename Comp, typename Alloc>
void Init(std::multimap<K, T, Comp, Alloc>& I);

template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
void Init(std::unordered_map<K, T, Hash, Pred, Alloc>& I);

template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
void Init(std::unordered_multimap<K, T, Hash, Pred, Alloc>& I);

template <typename Iter>
void InitMap(Iter& I)
{
    auto size = size_t(rand() % 100);
    for (size_t i = 0; i < size; ++i)
    {
        pair<typename Iter::key_type, typename Iter::mapped_type> V;
        V.first = i;
        Init(V.second);
        IteratableAdd(I, V);
    }
}

template <typename K, typename T, typename Comp, typename Alloc>
void Init(std::map<K, T, Comp, Alloc>& I)
{
    InitMap(I);
}

template <typename K, typename T, typename Comp, typename Alloc>
void Init(std::multimap<K, T, Comp, Alloc>& I)
{
    InitMap(I);
}

template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
void Init(std::unordered_map<K, T, Hash, Pred, Alloc>& I)
{
    InitMap(I);
}

template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
void Init(std::unordered_multimap<K, T, Hash, Pred, Alloc>& I)
{
    InitMap(I);
}

template <typename K, typename T>
void Init(pair<K, T>& pair)
{
    Init(pair.first);
    Init(pair.second);
}

template <typename Iter>
void TestBasic()
{
    Iter I;
    Init(I);
    Iter copy = I;

    auto S = Serio::serialize(I);
    I = 0;
    Serio::deserialize(S, I);

    EXPECT_EQ(I, copy);
}

template <typename Iter>
void TestIter(Iter I)
{
    Init(I);
    Iter copy = I;

    auto S = Serio::serialize(I);
    I.clear();
    Serio::deserialize(S, I);

    EXPECT_EQ(I, copy);
}

template <typename T>
void TestValArray()
{
    valarray<T> I;
    Init(I);
    valarray<T> copy = I;

    auto S = Serio::serialize(I);
    I.resize(0);
    Serio::deserialize(S, I);

    valarray<bool> V = (I == copy);
    bool x = true;

    for (const auto& B : V)
        if (!B) x = false;

    EXPECT_EQ(x, true);
}

struct A
{
    double a;
    double b;

    bool operator==(const A& other) const
    {
        auto x = abs(this->a - other.a) < numeric_limits<double>::epsilon();
        auto y = abs(this->b - other.b) < numeric_limits<double>::epsilon();
        return x && y;
    }

    SERIO_REGISTER(a, b)
};

struct B
{
    double a;
    double b;
    A c;

    bool operator==(const B& other) const
    {
        auto x = abs(this->a - other.a) < numeric_limits<double>::epsilon();
        auto y = abs(this->b - other.b) < numeric_limits<double>::epsilon();
        return x && y && this->c == other.c;
    }

    SERIO_REGISTER(a, b, c)
};

struct C
{
    double a;

    bool operator==(const C& other) const
    {
        return abs(this->a - other.a) < numeric_limits<double>::epsilon();
    }

    SERIO_REGISTER(a)
};

struct D : C
{
    double b;

    bool operator==(const D& other) const
    {
        auto x = abs(this->a - other.a) < numeric_limits<double>::epsilon();
        auto y = abs(this->b - other.b) < numeric_limits<double>::epsilon();
        return x && y;
    }

    SERIO_REGISTER(a, b)
};

void Init(A& a)
{
    Init(a.a);
    Init(a.b);
}

void Init(B& a)
{
    Init(a.a);
    Init(a.b);
    Init(a.c);
}

void Init(D& a)
{
    Init(a.a);
    Init(a.b);
}

void TestA()
{
    A I;
    Init(I);
    A copy = I;

    auto S = Serio::serialize(I);
    I.a = 0;
    I.b = 0;
    Serio::deserialize(S, I);

    EXPECT_EQ(I, copy);
}

void TestB()
{
    B I;
    Init(I);
    B copy = I;

    auto S = Serio::serialize(I);
    I.a = 0;
    I.b = 0;
    I.c.a = 0;
    I.c.b = 0;
    Serio::deserialize(S, I);

    EXPECT_EQ(I, copy);
}

void TestD()
{
    D I;
    Init(I);
    D copy = I;

    auto S = Serio::serialize(I);
    I.a = 0;
    I.b = 0;
    Serio::deserialize(S, I);

    EXPECT_EQ(I, copy);
}

TEST(Basic, Serio)
{
    TestBasic<char>();
    TestBasic<signed char>();
    TestBasic<short>();
    TestBasic<int>();
    TestBasic<long>();
    TestBasic<long long>();
    TestBasic<unsigned char>();
    TestBasic<unsigned short>();
    TestBasic<unsigned int>();
    TestBasic<unsigned long>();
    TestBasic<unsigned long long>();
    TestBasic<wchar_t>();
    TestBasic<bool>();
    TestBasic<float>();
    TestBasic<double>();
#ifdef __SIZEOF_INT128__
    TestBasic<long double>();
#endif
}

TEST(String, Serio) { TestIter(string()); }

CREATE_ITER_TEST(Vector, vector);
CREATE_ITER_TEST(Deque, deque);
CREATE_ITER_TEST(List, list);
CREATE_ITER_TEST(ForwardList, forward_list);

CREATE_ITER_TEST(Set, set);
CREATE_ITER_TEST(MultiSet, multiset);
CREATE_ITER_TEST_0_DEPTH(UnorderedSet, unordered_set);
CREATE_ITER_TEST_0_DEPTH(UnorderedMultiSet, unordered_multiset);

CREATE_MAP_TEST(Map, map);
CREATE_MAP_TEST(MultiMap, multimap);
CREATE_MAP_TEST(UnorderedMap, unordered_map);
CREATE_MAP_TEST(UnorderedMultiMap, unordered_multimap);

TEST(ValArray, Serio)
{
    TestValArray<short>();
    TestValArray<int>();
    TestValArray<long>();
    TestValArray<double>();
}

TEST(Custom, Serio)
{
    TestA();
    TestB();
    TestD();

    TestIter(vector<B>());
    TestIter(vector<vector<B>>());
    TestIter(list<B>());
    TestIter(list<list<B>>());
    TestIter(deque<B>());
    TestIter(deque<deque<B>>());

    TestIter(vector<D>());
    TestIter(vector<vector<D>>());
    TestIter(list<D>());
    TestIter(list<list<D>>());
    TestIter(deque<D>());
    TestIter(deque<deque<D>>());
}

TEST(Complex, Serio)
{
    TestBasic<complex<int>>();
    TestBasic<complex<double>>();
    TestIter(vector<complex<double>>());
    TestIter(vector<vector<complex<double>>>());
    TestIter(list<complex<double>>());
    TestIter(list<list<complex<double>>>());
    TestIter(deque<complex<double>>());
    TestIter(deque<deque<complex<double>>>());
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    srand(time(nullptr));
    return RUN_ALL_TESTS();
}
