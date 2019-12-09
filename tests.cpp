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
#include <codecvt>
#include <ctime>
#include <limits>
#include <locale>
#include <random>

struct A
{
    double a;
    double b;

    bool operator==(const A& other) const
    {
        auto x = std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
        auto y = std::abs(this->b - other.b) < std::numeric_limits<double>::epsilon();
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
        auto x = std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
        auto y = std::abs(this->b - other.b) < std::numeric_limits<double>::epsilon();
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
        return std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
    }
    bool operator!=(const C& other) const { return !(*this == other); }

    SERIO_REGISTER(a)
};
struct D : C
{
    double b;

    bool operator==(const D& other) const
    {
        auto x = std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
        auto y = std::abs(this->b - other.b) < std::numeric_limits<double>::epsilon();
        return x && y;
    }
    bool operator!=(const D& other) const { return !(*this == other); }

    SERIO_REGISTER(a, b)
};

class INIT
{
    template <typename T, typename Seq>
    static void add(std::queue<T, Seq>& I, T& value)
    {
        I.push(std::move(value));
    }
    template <typename T, typename Seq>
    static void add(std::stack<T, Seq>& I, T& value)
    {
        I.push(std::move(value));
    }
    template <typename T, typename Seq, typename Comp>
    static void add(std::priority_queue<T, Seq, Comp>& I, T& value)
    {
        I.push(std::move(value));
    }
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
        auto size = 10 + size_t(rand() % 90);
        for (size_t i = 0; i < size; ++i)
        {
            std::pair<typename Iter::key_type, typename Iter::mapped_type> V;
            init(V);
            add(I, V);
        }
    }

public:
    template <typename T>
    static typename std::enable_if<std::is_arithmetic<T>::value, void>::type init(T& V)
    {
        V = rand() % int(std::numeric_limits<T>::max());
        if (std::is_same<bool, T>::value && rand() % 2) V = !V;
    }
    template <typename Iter>
    static typename std::enable_if<std::is_class<Iter>::value, void>::type init(Iter& I)
    {
        auto size = 10 + size_t(rand() % 90);
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
        auto size = 10 + size_t(rand() % 90);
        I = std::valarray<T>(T(), size);
        for (auto& value : I) init(value);
    }
    template <typename T, size_t N>
    static void init(std::array<T, N>& I)
    {
        for (auto& value : I) init(value);
    }

    template <typename T>
    static void init(std::complex<T>& C)
    {
        T real, imag;
        init(real);
        init(imag);
        C = std::complex<T>(real, imag);
    }
    template <typename K, typename T>
    static void init(std::pair<K, T>& pair)
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

    template <typename T>
    static void init(std::shared_ptr<T>& I)
    {
        auto* value = new T();
        init(*value);
        I.reset(value);
    }
    template <typename T, typename Deleter>
    static void init(std::unique_ptr<T, Deleter>& I)
    {
        auto* value = new T();
        init(*value);
        I.reset(value);
    }
    static void init(std::chrono::steady_clock::time_point& I)
    {
        I = std::chrono::steady_clock::now();
    }

    template <size_t N>
    static void init(std::bitset<N>& I)
    {
        for (size_t i = 0; i < N; ++i)
        {
            bool B;
            init(B);
            I[i] = B;
        }
    }

    template <typename T>
    static void init(std::tuple<T, T, T>& I)
    {
        init(std::get<0>(I));
        init(std::get<1>(I));
        init(std::get<2>(I));
    }

    template <typename T, size_t S>
    static void init(Serio::Array<T, S>& I)
    {
        for (size_t i = 0; i < S; ++i) init(I.data[i]);
    }

#if __cplusplus >= 201703L
    template <typename T>
    static void init(std::optional<T>& I)
    {
        T value;
        init(value);
        I.emplace(std::move(value));
    }
    static void init(std::variant<int, double, std::string>& I)
    {
        auto index = rand() % 3;
        if (index == 0)
        {
            int V;
            init(V);
            I = V;
        }
        else if (index == 1)
        {
            double V;
            init(V);
            I = V;
        }
        else if (index == 2)
        {
            std::string V;
            init(V);
            I = V;
        }
    }
#endif
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
    std::ofstream file("temp");
    ASSERT_TRUE(file.is_open());
    Serio::write(&file, std::forward<Ts>(ts)...);
}
template <typename... Ts>
void load2(Ts&&... ts)
{
    std::ifstream file("temp");
    ASSERT_TRUE(file.is_open());
    Serio::read(&file, std::forward<Ts>(ts)...);
}

template <typename... Ts>
void save3(Ts&&... ts)
{
    Serio::save("temp", std::forward<Ts>(ts)...);
}
template <typename... Ts>
void load3(Ts&&... ts)
{
    Serio::load("temp", std::forward<Ts>(ts)...);
}

template <typename T>
void compare(const T& value1, const T& value2)
{
    EXPECT_EQ(value1, value2);
}
template <typename T>
void compare(const std::valarray<T>& value1, const std::valarray<T>& value2)
{
    if (value1.size() == 0 && value2.size() == 0) return;
    EXPECT_TRUE((value1 == value2).min());
}
template <typename T, typename Seq, typename Comp>
void compare(std::priority_queue<T, Seq, Comp> value1, std::priority_queue<T, Seq, Comp> value2)
{
    while (!value1.empty() && !value2.empty())
    {
        compare(value1.top(), value2.top());
        value1.pop();
        value2.pop();
    }
    EXPECT_EQ(value1.empty(), value2.empty());
}
template <typename T>
void compare(const std::shared_ptr<T>& value1, const std::shared_ptr<T>& value2)
{
    if (!value1 && !value2) return;
    compare(*value1.get(), *value2.get());
}
template <typename T, typename Deleter>
void compare(const std::unique_ptr<T, Deleter>& value1, const std::unique_ptr<T, Deleter>& value2)
{
    if (!value1 && !value2) return;
    compare(*value1.get(), *value2.get());
}
template <typename T, size_t S>
void compare(const Serio::Array<T, S>& value1, const Serio::Array<T, S>& value2)
{
    for (size_t i = 0; i < S; ++i) compare(value1.data[i], value2.data[i]);
}
template <typename K, typename T>
void compare(const std::multimap<K, T>& value1, const std::multimap<K, T>& value2)
{
    // TODO look into this
    using U = std::unordered_multimap<K, T>;
    EXPECT_EQ(U(value1.begin(), value1.end()), U(value2.begin(), value2.end()));
}

std::basic_ostream<char>& operator<<(std::basic_ostream<char>& O, const std::wstring& S)
{
    return O << std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(S);
}

template <typename T>
struct Process
{
    Process()
    {
        T value1 = T(), value2 = T();
        save1(value1);
        load1(value2);
        compare(value1, value2);
        save2(value1);
        load2(value2);
        compare(value1, value2);
        save3(value1);
        load3(value2);
        compare(value1, value2);

        INIT::init(value1);
        save1(value1);
        load1(value2);
        compare(value1, value2);
        save2(value1);
        load2(value2);
        compare(value1, value2);
        save3(value1);
        load3(value2);
        compare(value1, value2);
    }
};

using BasicTypes =
    ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int, long,
                     long long, unsigned char, unsigned short, unsigned int, unsigned long,
                     unsigned long long, float, double, long double>;
using FullTypes =
    ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int, long,
                     long long, unsigned char, unsigned short, unsigned int, unsigned long,
                     unsigned long long, float, double, long double, A, B, D, std::complex<int>,
                     std::complex<float>, std::chrono::steady_clock::time_point, std::bitset<50>,
                     std::string, std::wstring>;

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

CREATE_ITER_TEST_0(Type1, PQueue, std::priority_queue);
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
TYPED_TEST(Type2, Tuple) { Process<std::pair<TypeParam, TypeParam>>(); }
TYPED_TEST(Type2, Pair) { Process<std::tuple<TypeParam, TypeParam, TypeParam>>(); }
CREATE_ITER_TEST(Type2, Vector, std::vector);
CREATE_ITER_TEST(Type2, List, std::list);
CREATE_ITER_TEST(Type2, Deque, std::deque);
CREATE_ITER_TEST_0(Type2, Queue, std::queue);
CREATE_ITER_TEST_0(Type2, Stack, std::stack);
CREATE_ITER_TEST(Type2, ForwardList, std::forward_list);
TYPED_TEST(Type2, ShapredPtr) { Process<std::shared_ptr<TypeParam>>(); }
TYPED_TEST(Type2, UniquePtr) { Process<std::unique_ptr<TypeParam>>(); }

template <typename T>
using Array = std::array<T, 50>;
CREATE_ITER_TEST(Type2, Array, Array);

TYPED_TEST(Type2, RawArray)
{
    TypeParam V1[50], V2[50];
    Serio::Array<TypeParam, 50> value1(V1), value2(V2);
    INIT::init(value1);
    save1(value1);
    load1(value2);
    compare(value1, value2);
    save2(value1);
    load2(value2);
    compare(value1, value2);
    save3(value1);
    load3(value2);
    compare(value1, value2);
}

#if __cplusplus >= 201703L
TYPED_TEST(Type2, Optional) { Process<std::optional<TypeParam>>(); }
TEST(Variant, Test) { Process<std::variant<int, double, std::string>>(); }
#endif

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    srand(static_cast<unsigned int>(time(nullptr)));
    return RUN_ALL_TESTS();
}
