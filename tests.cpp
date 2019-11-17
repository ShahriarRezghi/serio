/*
  Copyright (c) 2019, Shahriar Rezghi
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of Serio nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <gtest/gtest-typed-test.h>
#include <gtest/gtest.h>
#include <serio.h>

#include <cfloat>
#include <cmath>
#include <ctime>
#include <limits>
#include <random>

using namespace std;

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
    bool operator!=(const A& other) const { return !(*this == other); }

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
    bool operator!=(const B& other) const { return !(*this == other); }

    SERIO_REGISTER(a, b, c)
};
struct C
{
    double a;

    bool operator==(const C& other) const
    {
        return abs(this->a - other.a) < numeric_limits<double>::epsilon();
    }
    bool operator!=(const C& other) const { return !(*this == other); }

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
    bool operator!=(const D& other) const { return !(*this == other); }

    SERIO_REGISTER(a, b)
};

class INIT
{
    template <typename T, typename Alloc>
    static void add(std::vector<T, Alloc>& I, T& value)
    {
        I.push_back(std::move(value));
    }
    template <typename T, typename Alloc>
    static void add(std::list<T, Alloc>& I, T& value)
    {
        I.push_back(std::move(value));
    }
    template <typename T, typename Alloc>
    static void add(std::deque<T, Alloc>& I, T& value)
    {
        I.push_back(std::move(value));
    }
    template <typename T, typename Alloc>
    static void add(std::forward_list<T, Alloc>& I, T& value)
    {
        I.emplace_after(I.before_begin(), std::move(value));
    }
    template <typename T, typename Comp, typename Alloc>
    static void add(std::set<T, Comp, Alloc>& I, T& value)
    {
        I.emplace(std::move(value));
    }
    template <typename T, typename Comp, typename Alloc>
    static void add(std::multiset<T, Comp, Alloc>& I, T& value)
    {
        I.emplace(std::move(value));
    }
    template <typename T, typename Hash, typename Pred, typename Alloc>
    static void add(std::unordered_set<T, Hash, Pred, Alloc>& I, T& value)
    {
        I.emplace(std::move(value));
    }
    template <typename T, typename Hash, typename Pred, typename Alloc>
    static void add(std::unordered_multiset<T, Hash, Pred, Alloc>& I, T& value)
    {
        I.emplace(std::move(value));
    }
    template <typename K, typename T, typename Comp, typename Alloc>
    static void add(std::map<K, T, Comp, Alloc>& I, std::pair<K, T>& value)
    {
        I.emplace(std::move(value));
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    static void add(std::unordered_map<K, T, Hash, Pred, Alloc>& I, std::pair<K, T>& value)
    {
        I.emplace(std::move(value));
    }
    template <typename K, typename T, typename Comp, typename Alloc>
    static void add(std::multimap<K, T, Comp, Alloc>& I, std::pair<K, T>& value)
    {
        I.emplace(std::move(value));
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    static void add(std::unordered_multimap<K, T, Hash, Pred, Alloc>& I, std::pair<K, T>& value)
    {
        I.emplace(std::move(value));
    }
    template <typename T, typename Traits, typename Alloc>
    static void add(std::basic_string<T, Traits, Alloc>& I, T& value)
    {
        I.push_back(std::move(value));
    }

    template <typename Iter>
    static void initMap(Iter& I)
    {
        auto size = size_t(rand() % 100);
        for (size_t i = 0; i < size; ++i)
        {
            pair<typename Iter::key_type, typename Iter::mapped_type> V;
            V.first = typename Iter::key_type(i);
            init(V.second);
            add(I, V);
        }
    }

public:
    template <typename T>
    static typename std::enable_if<std::is_arithmetic<T>::value, void>::type init(T& V)
    {
        V = T((rand() / double(RAND_MAX) - .5) * pow(2, sizeof(T) * 8));
    }
    template <typename Iter>
    static typename std::enable_if<std::is_class<Iter>::value, void>::type init(Iter& I)
    {
        auto size = size_t(rand() % 100);
        for (size_t i = 0; i < size; ++i)
        {
            auto V = typename Iter::value_type();
            init(V);
            add(I, V);
        }
    }
    template <typename T>
    static void init(std::valarray<T>& I)
    {
        auto size = size_t(rand() % 100);
        I = std::valarray<T>(T(), size);
        for (auto& value : I) init(value);
    }

    template <typename T>
    static void init(complex<T>& C)
    {
        T real, imag;
        init(real);
        init(imag);
        C = complex<T>(real, imag);
    }
    template <typename K, typename T>
    static void init(pair<K, T>& pair)
    {
        init(pair.first);
        init(pair.second);
    }

    static void init(A& a)
    {
        init(a.a);
        init(a.b);
    }
    static void init(B& a)
    {
        init(a.a);
        init(a.b);
        init(a.c);
    }
    static void init(D& a)
    {
        init(a.a);
        init(a.b);
    }

    template <typename K, typename T, typename Comp, typename Alloc>
    static void init(std::map<K, T, Comp, Alloc>& I)
    {
        initMap(I);
    }
    template <typename K, typename T, typename Comp, typename Alloc>
    static void init(std::multimap<K, T, Comp, Alloc>& I)
    {
        initMap(I);
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    static void init(std::unordered_map<K, T, Hash, Pred, Alloc>& I)
    {
        initMap(I);
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    static void init(std::unordered_multimap<K, T, Hash, Pred, Alloc>& I)
    {
        initMap(I);
    }
};

static Serio::ByteArray temporary;

template <typename... Ts>
void save1(Ts&&... ts)
{
    temporary = Serio::serialize(std::forward<Ts>(ts)...);
}
template <typename... Ts>
void load1(Ts&&... ts)
{
    ASSERT_EQ(Serio::deserialize(temporary, std::forward<Ts>(ts)...), temporary.size());
}

template <typename... Ts>
void save2(Ts&&... ts)
{
    ofstream file("temp");
    ASSERT_TRUE(file.is_open());
    Serio::streamSerialize(&file, std::forward<Ts>(ts)...);
}
template <typename... Ts>
void load2(Ts&&... ts)
{
    ifstream file("temp");
    ASSERT_TRUE(file.is_open());
    Serio::streamDeserialize(&file, std::forward<Ts>(ts)...);
}

template <typename T>
struct Process
{
    Process()
    {
        T value1, value2;
        INIT::init(value1);
        save1(value1);
        load1(value2);
        EXPECT_EQ(value1, value2);
        save2(value1);
        load2(value2);
        EXPECT_EQ(value1, value2);
    }
};
template <typename T>
struct Process<std::valarray<T>>
{
    Process()
    {
        std::valarray<T> value1, value2;
        INIT::init(value1);
        save1(value1);
        load1(value2);
        EXPECT_TRUE((value1.size() == 0 && value2.size() == 0) || (value1 == value2).min());
        save2(value1);
        load2(value2);
        EXPECT_TRUE((value1.size() == 0 && value2.size() == 0) || (value1 == value2).min());
    }
};

#ifdef __SIZEOF_INT128__
using BasicTypes =
    ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int, long,
                     long long, unsigned char, unsigned short, unsigned int, unsigned long,
                     unsigned long long, float, double, long double>;
using FullTypes = ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int,
                                   long, long long, unsigned char, unsigned short, unsigned int,
                                   unsigned long, unsigned long long, float, double, long double, A,
                                   B, D, std::complex<int>, std::complex<float>>;
#else
using BasicTypes = ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short,
                                    int, long, long long, unsigned char, unsigned short,
                                    unsigned int, unsigned long, unsigned long long, float, double>;
using FullTypes = ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int,
                                   long, long long, unsigned char, unsigned short, unsigned int,
                                   unsigned long, unsigned long long, float, double, A, B, D,
                                   std::complex<int>, std::complex<float>>;
#endif

#define CREATE_ITER_TEST_0(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth0) { Process<TYPE<TypeParam>>(); }

#define CREATE_ITER_TEST_1(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth1) { Process<TYPE<TYPE<TypeParam>>>(); }

#define CREATE_ITER_TEST(TEST, NAME, TYPE) \
    CREATE_ITER_TEST_0(TEST, NAME, TYPE)   \
    CREATE_ITER_TEST_1(TEST, NAME, TYPE)

#define CREATE_MAP_TEST_0(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth0) { Process<TYPE<TypeParam, TypeParam>>(); }

#define CREATE_MAP_TEST_1(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth1) { Process<TYPE<TypeParam, TYPE<TypeParam, TypeParam>>>(); }

#define CREATE_MAP_TEST(TEST, NAME, TYPE) \
    CREATE_MAP_TEST_0(TEST, NAME, TYPE)   \
    CREATE_MAP_TEST_1(TEST, NAME, TYPE)

template <typename T>
struct Type1 : public ::testing::Test
{
};
template <typename T>
struct Type2 : public ::testing::Test
{
};
TYPED_TEST_CASE(Type1, BasicTypes);
TYPED_TEST_CASE(Type2, FullTypes);

TYPED_TEST(Type1, Complex) { Process<std::complex<TypeParam>>(); }
CREATE_ITER_TEST_0(Type1, ValArray, std::valarray)
CREATE_ITER_TEST(Type1, Set, std::set);
CREATE_ITER_TEST(Type1, Multiset, std::multiset);
CREATE_ITER_TEST_0(Type1, Unorderedset, std::unordered_set);
CREATE_ITER_TEST_0(Type1, UnorderedMultiset, std::unordered_multiset);
CREATE_MAP_TEST(Type1, Map, std::map);
CREATE_MAP_TEST_0(Type1, Multimap, std::multimap);
CREATE_MAP_TEST(Type1, UnorderedMap, std::unordered_map);
CREATE_MAP_TEST(Type1, UnorderedMultimap, std::unordered_multimap);

TYPED_TEST(Type2, Basic) { Process<TypeParam>(); }
CREATE_ITER_TEST(Type2, Vector, std::vector);
CREATE_ITER_TEST(Type2, List, std::list);
CREATE_ITER_TEST(Type2, Deque, std::deque);
CREATE_ITER_TEST(Type2, ForwardList, std::forward_list);

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    srand(static_cast<unsigned int>(time(nullptr)));
    return RUN_ALL_TESTS();
}
