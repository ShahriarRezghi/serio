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

using namespace Serio;

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

struct Generator
{
    template <typename T>
    T get()
    {
        T value;
        *this >> value;
        return value;
    }

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, Generator&>::type operator>>(
        T& value)
    {
        value = rand() % int(std::numeric_limits<T>::max());
        if (std::is_same<bool, T>::value && rand() % 2) value = !value;
        return *this;
    }
    template <size_t N>
    Generator& operator>>(std::bitset<N>& value)
    {
        for (size_t i = 0; i < N; ++i) value[i] = this->get<bool>();
        return *this;
    }
    template <typename T>
    typename std::enable_if<IsAssignable<T>::value, Generator&>::type operator>>(T& value)
    {
        value.resize(rand() % 100);
        for (auto& item : value) *this >> item;
        return *this;
    }
    template <typename T>
    typename std::enable_if<IsIteratable<T>::value, Generator&>::type operator>>(T& value)
    {
        using Type = typename ValueType<T>::Type;
        value.clear();
        auto it = value.begin();
        Size size = rand() % 100;
        for (Size i = 0; i < size; ++i) it = value.emplace_hint(it, this->get<Type>());
        return *this;
    }
    template <typename T>
    typename std::enable_if<IsPointer<T>::value, Generator&>::type operator>>(T& value)
    {
        if (this->get<bool>())
        {
            auto* item = new typename T::element_type();
            *this >> *item;
            value.reset(item);
        }
        else
            value.reset();

        return *this;
    }
    template <typename... Ts>
    Generator& operator>>(std::vector<bool, Ts...>& value)
    {
        value.resize(rand() % 100);
        for (size_t i = 0; i < value.size(); ++i) value[i] = get<bool>();
        return *this;
    }
    template <typename T, size_t N>
    Generator& operator>>(std::array<T, N>& value)
    {
        for (auto& item : value) *this >> item;
        return *this;
    }
    template <typename T, size_t S>
    Generator& operator>>(Array<T, S> value)
    {
        for (size_t i = 0; i < S; ++i) *this >> value.data[i];
        return *this;
    }
    template <typename... Ts>
    Generator& operator>>(std::queue<Ts...>& value)
    {
        return *this >> reinterpret_cast<Queue<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Generator& operator>>(std::stack<Ts...>& value)
    {
        return *this >> reinterpret_cast<Stack<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Generator& operator>>(std::priority_queue<Ts...>& value)
    {
        return *this >> reinterpret_cast<PQueue<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Generator& operator>>(std::pair<Ts...>& value)
    {
        return *this >> value.first >> value.second;
    }
    template <typename... Ts>
    Generator& operator>>(std::tuple<Ts...>& value)
    {
        Tuple<sizeof...(Ts) - 1>::deserialize(*this, value);
        return *this;
    }
    template <typename T>
    Generator& operator>>(std::complex<T>& value)
    {
        value.real(get<T>());
        value.imag(get<T>());
        return *this;
    }
    template <typename... Ts>
    Generator& operator>>(std::chrono::time_point<Ts...>& C)
    {
        using rep = typename std::chrono::time_point<Ts...>::rep;
        return *this >> reinterpret_cast<rep&>(C);
    }
    Generator& operator>>(A& value)  //
    {
        return *this >> value.a >> value.b;
    }
    Generator& operator>>(B& value)  //
    {
        return *this >> value.a >> value.b >> value.c;
    }
    Generator& operator>>(C& value)  //
    {
        return *this >> value.a;
    }
    Generator& operator>>(D& value)  //
    {
        return *this >> value.a >> value.b;
    }

#if __cplusplus >= 201703L
    template <typename T>
    Generator& operator>>(std::optional<T>& C)
    {
        if (this->get<bool>())
            C.emplace(this->get<T>());
        else
            C.reset();

        return *this;
    }
    template <typename... Ts>
    Generator& operator>>(std::variant<Ts...>& value)
    {
        auto index = this->get<Size>();
        Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(*this, index, value);
        return *this;
    }
    Generator& operator>>(std::monostate& value)
    {
        (void)value;
        return *this;
    }
#endif
};

template <typename Type>
struct Process
{
    std::basic_string<char> temporary;

    template <typename T>
    typename std::enable_if<!IsPointer<T>::value>::type compare(const T& value1, const T& value2)
    {
        EXPECT_EQ(value1, value2);
    }
    template <typename T>
    void compare(const std::valarray<T>& value1, const std::valarray<T>& value2)
    {
        if (value1.size() == 0 && value2.size() == 0) return;
        EXPECT_TRUE((value1 == value2).min());
    }
    template <typename... Ts>
    void compare(std::priority_queue<Ts...> value1, std::priority_queue<Ts...> value2)
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
    typename std::enable_if<IsPointer<T>::value>::type compare(const T& value1, const T& value2)
    {
        if (!value1 && !value2) return;
        compare(*value1.get(), *value2.get());
    }
    template <typename K, typename T>
    void compare(const std::multimap<K, T>& value1, const std::multimap<K, T>& value2)
    {
        using U = std::unordered_multimap<K, T>;
        EXPECT_EQ(U(value1.begin(), value1.end()), U(value2.begin(), value2.end()));
    }
    template <typename T, size_t S>
    void compare(const Serio::Array<T, S>& value1, const Serio::Array<T, S>& value2)
    {
        for (size_t i = 0; i < S; ++i) compare(value1.data[i], value2.data[i]);
    }

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
        std::fstream file("temp", std::ios::binary | std::ios::out);
        ASSERT_TRUE(file.is_open());
        Serio::write(&file, std::forward<Ts>(ts)...);
    }
    template <typename... Ts>
    void load2(Ts&&... ts)
    {
        std::fstream file("temp", std::ios::binary | std::ios::in);
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

    Process()
    {
        Type value1 = {}, value2 = {};
        save1(value1);
        load1(value2);
        compare(value1, value2);
        save2(value1);
        load2(value2);
        compare(value1, value2);
        save3(value1);
        load3(value2);
        compare(value1, value2);

        Generator() >> value1;
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
    Process(Type value1, Type value2)
    {
        Generator() >> value1;
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
    ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int, long, long long, unsigned char,
                     unsigned short, unsigned int, unsigned long, unsigned long long, float, double, long double>;
using FullTypes = ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int, long, long long,
                                   unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long,
                                   float, double, long double, A, B, D, std::complex<int>, std::complex<float>,
                                   std::chrono::steady_clock::time_point, std::bitset<50>, std::string, std::wstring>;

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
TYPED_TEST_SUITE(Type1, BasicTypes);
TYPED_TEST_SUITE(Type2, FullTypes);

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
TYPED_TEST(Type2, Pair) { Process<std::pair<TypeParam, TypeParam>>(); }
TYPED_TEST(Type2, Tuple) { Process<std::tuple<TypeParam, TypeParam, TypeParam>>(); }
CREATE_ITER_TEST(Type2, Vector, std::vector);
CREATE_ITER_TEST(Type2, List, std::list);
CREATE_ITER_TEST(Type2, Deque, std::deque);
CREATE_ITER_TEST_0(Type2, Queue, std::queue);
CREATE_ITER_TEST_0(Type2, Stack, std::stack);
CREATE_ITER_TEST(Type2, ForwardList, std::forward_list);
TYPED_TEST(Type2, ShapredPtr) { Process<std::shared_ptr<TypeParam>>(); }
TYPED_TEST(Type2, UniquePtr) { Process<std::unique_ptr<TypeParam>>(); }

template <typename T>
using StdArray = std::array<T, 50>;
CREATE_ITER_TEST(Type2, Array, StdArray);

TYPED_TEST(Type2, RawArray)
{
    TypeParam V1[50], V2[50];
    Serio::Array<TypeParam, 50> value1(V1), value2(V2);
    Process<Serio::Array<TypeParam, 50>>(value1, value2);
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
