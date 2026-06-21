// BSD 3-Clause License
//
// Copyright (c) 2019-2026, Shahriar Rezghi <shahriar.rezghi.sh@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <gtest/gtest-typed-test.h>
#include <gtest/gtest.h>
#include <serio/serio.h>

#ifdef SERIO_TEST_BOOST
#include <serio/boost.h>
#endif

#ifdef SERIO_TEST_QT
#include <serio/qt.h>
#endif

#include <cfloat>
#include <cmath>
#include <codecvt>
#include <ctime>
#include <fstream>
#include <limits>
#include <locale>
#include <random>

using Serio::Size;

#define MAX_SIZE 100

namespace std
{
inline ostream& operator<<(ostream& os, char32_t ch) { return os << static_cast<uint32_t>(ch); }
inline ostream& operator<<(ostream& os, char16_t ch) { return os << static_cast<uint16_t>(ch); }
inline ostream& operator<<(ostream& os, wchar_t ch) { return os << static_cast<uint16_t>(ch); }
}  // namespace std

struct A
{
    double a;
    double b;

    inline bool operator==(const A& other) const
    {
        auto x = std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
        auto y = std::abs(this->b - other.b) < std::numeric_limits<double>::epsilon();
        return x && y;
    }
    inline bool operator!=(const A& other) const { return !(*this == other); }

    SERIO_REGISTER(a, b)
};
struct B
{
    double a;
    double b;
    A c;

    inline bool operator==(const B& other) const
    {
        auto x = std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
        auto y = std::abs(this->b - other.b) < std::numeric_limits<double>::epsilon();
        return x && y && this->c == other.c;
    }
    inline bool operator!=(const B& other) const { return !(*this == other); }

    SERIO_REGISTER(a, b, c)
};
struct C
{
    double a;

    inline bool operator==(const C& other) const
    {
        return std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
    }
    inline bool operator!=(const C& other) const { return !(*this == other); }

    SERIO_REGISTER(a)
};
struct D : C
{
    double b;

    inline bool operator==(const D& other) const
    {
        auto x = std::abs(this->a - other.a) < std::numeric_limits<double>::epsilon();
        auto y = std::abs(this->b - other.b) < std::numeric_limits<double>::epsilon();
        return x && y;
    }
    inline bool operator!=(const D& other) const { return !(*this == other); }

    SERIO_REGISTER(a, b)
};
struct E
{
    bool b;
    int i;
    double d;
    std::string s;

    SERIO_FRIEND;

    template <typename Serializer>
    void _serialize(Serializer& C) const
    {
        C.process(Serio::nvp("b", b), Serio::nvp("i", i), Serio::nvp("d", d), Serio::nvp("s", s));
    }
    template <typename Deserializer>
    void _deserialize(Deserializer& C)
    {
        C.process(Serio::nvp("b", b), Serio::nvp("i", i), Serio::nvp("d", d), Serio::nvp("s", s));
    }

    template <typename Serializer>
    void _jserialize(Serializer& C) const
    {
        C.process(Serio::nvp("b", b), Serio::nvp("i", i), Serio::nvp("d", d), Serio::nvp("s", s));
    }
    template <typename Deserializer>
    void _jdeserialize(Deserializer& C)
    {
        C.process(Serio::nvp("b", b), Serio::nvp("i", i), Serio::nvp("d", d), Serio::nvp("s", s));
    }
};

template <typename T>
struct BString
{
    SERIO_REGISTER(Serio::binaryString(value))

    T value;

    template <typename F>
    bool operator==(const BString<F>& other) const
    {
        return value == other.value;
    }
};

inline char32_t random_unicode()
{
    struct temp
    {
        int64_t start, end, add;
    };

    std::vector<temp> list = {{0, 127, 0},
                              {128, 255, 0},
                              {256, 383, 0},
                              {384, 591, 0},
                              {592, 687, 0},
                              {688, 767, 0},
                              {768, 879, 0},
                              {880, 1023, 0},
                              {1024, 1279, 0},
                              {1280, 1319, 0},
                              {1320, 1409, 9},
                              {1410, 1509, 15},
                              {1510, 1765, 26},
                              {1766, 1845, 26},
                              {1846, 1893, 26},
                              {1894, 1943, 26},
                              {1944, 2002, 40},
                              {2003, 2065, 45},
                              {2066, 2096, 46},
                              {2097, 2224, 207},
                              {2225, 2347, 208},
                              {2348, 2464, 213},
                              {2465, 2577, 224},
                              {2578, 2696, 239},
                              {2697, 2817, 249},
                              {2818, 2944, 255},
                              {2945, 3057, 257},
                              {3058, 3183, 272},
                              {3184, 3298, 274},
                              {3299, 3389, 286},
                              {3390, 3482, 323},
                              {3483, 3701, 357},
                              {3702, 3861, 394},
                              {3862, 3954, 394},
                              {3955, 4210, 397},
                              {4211, 4591, 397},
                              {4592, 4617, 400},
                              {4618, 4702, 406},
                              {4703, 5342, 417},
                              {5343, 5371, 417},
                              {5372, 5452, 420},
                              {5453, 5473, 435},
                              {5474, 5496, 446},
                              {5497, 5516, 455},
                              {5517, 5536, 467},
                              {5537, 5658, 479},
                              {5659, 5829, 485},
                              {5830, 5899, 490},
                              {5900, 5979, 500},
                              {5980, 6016, 500},
                              {6017, 6112, 511},
                              {6113, 6144, 511},
                              {6145, 6176, 511},
                              {6177, 6318, 511},
                              {6319, 6443, 593},
                              {6444, 6501, 596},
                              {6502, 6565, 602},
                              {6566, 6645, 602},
                              {6646, 6693, 602},
                              {6694, 6728, 682},
                              {6729, 6856, 695},
                              {6857, 6920, 695},
                              {6921, 6984, 695},
                              {6985, 7240, 695},
                              {7241, 7495, 695},
                              {7496, 7607, 696},
                              {7608, 7652, 696},
                              {7653, 7678, 699},
                              {7679, 7711, 721},
                              {7712, 7791, 736},
                              {7792, 7849, 736},
                              {7850, 7961, 742},
                              {7962, 8217, 742},
                              {8218, 8461, 742},
                              {8462, 8500, 754},
                              {8501, 8511, 779},
                              {8512, 8671, 800},
                              {8672, 8799, 800},
                              {8800, 8831, 800},
                              {8832, 8927, 800},
                              {8928, 9183, 800},
                              {9184, 9374, 801},
                              {9375, 9422, 801},
                              {9423, 9438, 801},
                              {9439, 9694, 801},
                              {9695, 9822, 801},
                              {9823, 9950, 801},
                              {9951, 10206, 801},
                              {10207, 10296, 801},
                              {10297, 10391, 967},
                              {10392, 10423, 968},
                              {10424, 10551, 968},
                              {10552, 10589, 968},
                              {10590, 10669, 978},
                              {10670, 10764, 978},
                              {10765, 10796, 979},
                              {10797, 10846, 979},
                              {10847, 10962, 1057},
                              {10963, 11176, 1069},
                              {11177, 11188, 1095},
                              {11189, 11252, 1099},
                              {11253, 11347, 1100},
                              {11348, 11443, 1100},
                              {11444, 11484, 1105},
                              {11485, 11578, 1108},
                              {11579, 11594, 1109},
                              {11595, 11621, 1109},
                              {11622, 11657, 1114},
                              {11658, 11673, 1126},
                              {11674, 11928, 1126},
                              {11929, 12184, 1127},
                              {12185, 18766, 1127},
                              {18767, 18830, 1137},
                              {18831, 39770, 1137},
                              {39771, 40935, 1189},
                              {40936, 40990, 1192},
                              {40991, 41038, 1201},
                              {41039, 41338, 1201},
                              {41339, 41426, 1221},
                              {41427, 41514, 1229},
                              {41515, 41546, 1237},
                              {41547, 41770, 1237},
                              {41771, 41814, 1237},
                              {41815, 41824, 1241},
                              {41825, 41880, 1247},
                              {41881, 41970, 1255},
                              {41971, 41998, 1261},
                              {41999, 42046, 1265},
                              {42047, 42094, 1265},
                              {42095, 42123, 1265},
                              {42124, 42219, 1268},
                              {42220, 42315, 1300},
                              {42316, 42343, 1300},
                              {42344, 42439, 1304},
                              {42440, 42485, 1337},
                              {42486, 42543, 1482},
                              {42544, 53715, 1488},
                              {53716, 53791, 1500},
                              {53792, 54687, 1504},
                              {54688, 54815, 1504},
                              {54816, 55839, 1504},
                              {55840, 62239, 1504},
                              {62240, 62713, 1504},
                              {62714, 62793, 1542},
                              {62794, 63479, 1542},
                              {63480, 63495, 1544},
                              {63496, 63505, 1544},
                              {63506, 63512, 1550},
                              {63513, 63544, 1559},
                              {63545, 63572, 1559},
                              {63573, 63716, 1563},
                              {63717, 63954, 1564},
                              {63955, 63959, 1574},
                              {63960, 64053, 1576},
                              {64054, 64176, 1610},
                              {64177, 64240, 1615},
                              {64241, 64315, 1615},
                              {64316, 64327, 1620},
                              {64328, 64373, 1672},
                              {64374, 64402, 1802},
                              {64403, 64451, 1805},
                              {64452, 64487, 1852},
                              {64488, 64514, 1864},
                              {64515, 64546, 1917},
                              {64547, 64600, 1917},
                              {64601, 64680, 1959},
                              {64681, 64728, 1959},
                              {64729, 64770, 1959},
                              {64771, 64834, 2813},
                              {64835, 64866, 2813},
                              {64867, 64898, 2973},
                              {64899, 64930, 2973},
                              {64931, 65019, 3165},
                              {65020, 65051, 3172},
                              {65052, 65115, 3300},
                              {65116, 65147, 3300},
                              {65148, 65179, 3300},
                              {65180, 65252, 3428},
                              {65253, 65283, 3963},
                              {65284, 65395, 4348},
                              {65396, 65461, 4364},
                              {65462, 66340, 8266},
                              {66341, 66456, 8411},
                              {66457, 67527, 11367},
                              {67528, 68096, 24632},
                              {68097, 68098, 42495},
                              {68099, 68344, 50685},
                              {68345, 68566, 50695},
                              {68567, 68636, 50729},
                              {68637, 68723, 50915},
                              {68724, 68741, 50924},
                              {68742, 69765, 51066},
                              {69766, 69809, 57210},
                              {69810, 69909, 57214},
                              {69910, 69973, 57226},
                              {69974, 70229, 57258},
                              {70230, 70311, 57258},
                              {70312, 71079, 57432},
                              {71080, 71158, 57433},
                              {71159, 71228, 57481},
                              {71229, 71344, 57539},
                              {71345, 114055, 59727},
                              {114056, 118204, 59768},
                              {118205, 118426, 59779},
                              {118427, 118968, 76133},
                              {118969, 119095, 798536},
                              {119096, 119335, 798664},
                              {119336, 184869, 863704},
                              {184870, 250403, 863706}};

    int64_t number = rand() % 250404;

    auto comp = [](temp A, int64_t B) { return A.start < B; };
    auto it = std::lower_bound(list.begin(), list.end(), number, comp);
    return number + it->add;
}

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
        value = T(rand());
        if (std::is_floating_point<T>::value) value /= 1000;
        if (std::is_same<T, bool>::value) value = rand() % 2;
        return *this;
    }
    template <size_t N>
    Generator& operator>>(std::bitset<N>& value)
    {
        for (size_t i = 0; i < N; ++i) value[i] = this->get<bool>();
        return *this;
    }
    template <typename T>
    typename std::enable_if<Serio::Impl::IsResizable<T>::value, Generator&>::type operator>>(T& value)
    {
        value.resize(rand() % MAX_SIZE);
        for (auto& item : value) *this >> item;
        return *this;
    }
    template <typename T>
    typename std::enable_if<Serio::Impl::IsAppendable<T>::value, Generator&>::type operator>>(T& value)
    {
        using Type = typename Serio::Impl::ValueType<T>::Type;
        value.clear();
        auto it = value.begin();
        Size size = rand() % MAX_SIZE;
        for (Size i = 0; i < size; ++i) it = value.emplace_hint(it, this->get<Type>());
        return *this;
    }
    template <typename T>
    typename std::enable_if<Serio::Impl::IsPointer<T>::value, Generator&>::type operator>>(T& value)
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
        value.resize(rand() % MAX_SIZE);
        for (size_t i = 0; i < value.size(); ++i) value[i] = get<bool>();
        return *this;
    }
    template <typename... Ts>
    Generator& operator>>(std::basic_string<wchar_t, Ts...>& value)
    {
        std::u32string string;
        string.resize(rand() % MAX_SIZE);
        for (auto& c : string) c = rand() % 128;  // random_unicode();
        auto output = Serio::Impl::toutf8(string.data(), string.data() + string.size());
        Serio::Impl::fromutf8(output, value);
        // std::wcout << value << std::endl;
        return *this;
    }
    template <typename T, size_t N>
    Generator& operator>>(std::array<T, N>& value)
    {
        for (auto& item : value) *this >> item;
        return *this;
    }
    template <typename T, Size N>
    Generator& operator>>(Serio::StaticArrayView<T, N> value)
    {
        for (auto& item : value) *this >> item;
        return *this;
    }
    template <typename... Ts>
    Generator& operator>>(std::queue<Ts...>& value)
    {
        return *this >> reinterpret_cast<Serio::Impl::Queue<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Generator& operator>>(std::stack<Ts...>& value)
    {
        return *this >> reinterpret_cast<Serio::Impl::Stack<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Generator& operator>>(std::priority_queue<Ts...>& value)
    {
        return *this >> reinterpret_cast<Serio::Impl::PQueue<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Generator& operator>>(std::pair<Ts...>& value)
    {
        return *this >> value.first >> value.second;
    }
    template <typename... Ts>
    Generator& operator>>(std::tuple<Ts...>& value)
    {
        Serio::Impl::Tuple<sizeof...(Ts) - 1>::deserialize(*this, value);
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
    inline Generator& operator>>(A& value)  //
    {
        return *this >> value.a >> value.b;
    }
    inline Generator& operator>>(B& value)  //
    {
        return *this >> value.a >> value.b >> value.c;
    }
    inline Generator& operator>>(C& value)  //
    {
        return *this >> value.a;
    }
    inline Generator& operator>>(D& value)  //
    {
        return *this >> value.a >> value.b;
    }

    template <typename T>
    Generator& operator>>(BString<T>& value)
    {
        return *this >> value.value;
    }
    // template <typename T>
    // Generator& operator>>(std::wstring& value)
    // {
    //     std::u32string string(rand() % MAX_SIZE, 0);
    //     for (auto& c : value) c = char32_t(rand() % 0x10FFFF);
    //     auto temp = Serio::Impl::toutf8(string.data(), string.data() + string.size());
    //     Serio::Impl::fromutf8(temp, value);
    // }

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
        Serio::Impl::Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(*this, index, value);
        return *this;
    }
    inline Generator& operator>>(std::monostate& value)
    {
        (void)value;
        return *this;
    }
#endif

#ifdef SERIO_TEST_BOOST
    template <typename... Ts>
    Generator& operator>>(boost::tuple<Ts...>& value)
    {
        Serio::Impl::BoostTuple<sizeof...(Ts) - 1>::deserialize(*this, value);
        return *this;
    }
    template <typename T>
    Generator& operator>>(boost::optional<T>& C)
    {
        if (this->get<bool>())
            C.emplace(this->get<T>());
        else
            C.reset();

        return *this;
    }
    template <typename T, size_t N>
    Generator& operator>>(boost::array<T, N>& value)
    {
        for (auto& item : value) *this >> item;
        return *this;
    }
#endif

#ifdef SERIO_TEST_QT
    Generator& operator>>(QChar& value)
    {
        value = QChar(this->get<char16_t>());
        return *this;
    }
    inline Generator& operator>>(QStringList& value)
    {
        value.clear();
        Size size = rand() % MAX_SIZE;
        for (Size i = 0; i < size; ++i) value.append(this->get<QString>());
        return *this;
    }
    template <typename T>
    Generator& operator>>(QList<T>& value)
    {
        value.clear();
        Size size = rand() % MAX_SIZE;
        for (Size i = 0; i < size; ++i) value.append(this->get<T>());
        return *this;
    }
    template <typename T>
    Generator& operator>>(QSet<T>& value)
    {
        value.clear();
        Size size = rand() % MAX_SIZE;
        for (Size i = 0; i < size; ++i) value.insert(this->get<T>());
        return *this;
    }
    template <typename T, qsizetype N>
    Generator& operator>>(QVarLengthArray<T, N>& value)
    {
        value.clear();
        Size size = rand() % N;
        for (Size i = 0; i < size; ++i) value.append(this->get<T>());
        return *this;
    }
    template <typename T>
    typename std::enable_if<Serio::Impl::IsQtMap<T>::value, Generator&>::type operator>>(T& value)
    {
        using K = typename T::key_type;
        using V = typename T::mapped_type;

        value.clear();
        Size size = rand() % MAX_SIZE;
        for (Size i = 0; i < size; ++i)
        {
            value.insert(this->get<K>(), this->get<V>());
        }
        return *this;
    }
#endif
};

template <typename Type, Serio::Type ST>
struct Process
{
    std::string temporary;
    std::string password;

    template <typename T>
    typename std::enable_if<!Serio::Impl::IsPointer<T>::value>::type compare(const T& value1, const T& value2)
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
    typename std::enable_if<Serio::Impl::IsPointer<T>::value>::type compare(const T& value1, const T& value2)
    {
        if (!value1 && !value2) return;
        EXPECT_EQ(bool(value1), bool(value2));
        compare(*value1.get(), *value2.get());
    }
    template <typename K, typename T>
    void compare(const std::multimap<K, T>& value1, const std::multimap<K, T>& value2)
    {
        using U = std::unordered_multimap<K, T>;
        EXPECT_EQ(U(value1.begin(), value1.end()), U(value2.begin(), value2.end()));
    }

#ifdef SERIO_TEST_BOOST
    template <typename K, typename T>
    void compare(const boost::container::multimap<K, T>& value1, const boost::container::multimap<K, T>& value2)
    {
        using U = std::unordered_multimap<K, T>;
        EXPECT_EQ(U(value1.begin(), value1.end()), U(value2.begin(), value2.end()));
    }
    template <typename K, typename T>
    void compare(const boost::container::flat_multimap<K, T>& value1,
                 const boost::container::flat_multimap<K, T>& value2)
    {
        using U = std::unordered_multimap<K, T>;
        EXPECT_EQ(U(value1.begin(), value1.end()), U(value2.begin(), value2.end()));
    }
#endif

#ifdef SERIO_TEST_QT
    template <typename T>
    void compare(const QSet<T>& value1, const QSet<T>& value2)
    {
        using V = std::vector<T>;
        V v1(value1.begin(), value1.end());
        V v2(value2.begin(), value2.end());
        std::sort(v1.begin(), v1.end());
        std::sort(v2.begin(), v2.end());
        EXPECT_EQ(v1, v2);
    }
    template <typename K, typename T>
    void compare(const QMultiMap<K, T>& value1, const QMultiMap<K, T>& value2)
    {
        using U = std::unordered_multimap<K, T>;
        U v1, v2;
        for (auto i = value1.cbegin(), end = value1.cend(); i != end; ++i) v1.emplace(i.key(), i.value());
        for (auto i = value2.cbegin(), end = value2.cend(); i != end; ++i) v2.emplace(i.key(), i.value());
        EXPECT_EQ(v1, v2);
    }
    template <typename K, typename T>
    void compare(const QMultiHash<K, T>& value1, const QMultiHash<K, T>& value2)
    {
        using U = std::unordered_multimap<K, T>;
        U v1, v2;
        for (auto i = value1.cbegin(), end = value1.cend(); i != end; ++i) v1.emplace(i.key(), i.value());
        for (auto i = value2.cbegin(), end = value2.cend(); i != end; ++i) v2.emplace(i.key(), i.value());
        EXPECT_EQ(v1, v2);
    }
    template <typename K, typename T>
    void compare(const QHash<K, T>& value1, const QHash<K, T>& value2)
    {
        using U = std::unordered_map<K, T>;
        U v1, v2;
        for (auto i = value1.cbegin(), end = value1.cend(); i != end; ++i) v1[i.key()] = i.value();
        for (auto i = value2.cbegin(), end = value2.cend(); i != end; ++i) v2[i.key()] = i.value();
        EXPECT_EQ(v1, v2);
    }
#endif

    template <typename T, Size N>
    void compare(const Serio::StaticArrayView<T, N>& value1, const Serio::StaticArrayView<T, N>& value2)
    {
        compare(value1.size(), value2.size());
        for (size_t i = 0; i < value1.size(); ++i) compare(value1[i], value2[i]);
    }

    template <typename... Ts>
    void save1(Ts&&... ts)
    {
        Serio::SerializeOptions options;
        options.compactFrom = rand() % 2 == 0;
        options.enableChecksum = rand() % 2 == 0;

#ifdef SERIO_USE_COMPRESSION
        options.compressLevel = (rand() % 2 == 0) ? rand() % 23 : 0;
#endif

#ifdef SERIO_USE_ENCRYPTION
        options.encryptPassword.resize((rand() % 2 == 0) ? rand() % 17 : 0);
        for (auto& c : options.encryptPassword) c = rand() % 128;
        password = options.encryptPassword;
#endif

        temporary = Serio::serialize<ST>(options, std::forward<Ts>(ts)...);
    }
    template <typename... Ts>
    void load1(Ts&&... ts)
    {
        Serio::DeserializeOptions options;
        options.decryptPassword = password;
        Serio::deserialize<ST>(options, temporary, std::forward<Ts>(ts)...);
    }
    template <typename... Ts>
    void save2(Ts&&... ts)
    {
        std::fstream file("temp", std::ios::binary | std::ios::out);
        ASSERT_TRUE(file.is_open());
        Serio::write<ST>({}, file, std::forward<Ts>(ts)...);
    }
    template <typename... Ts>
    void load2(Ts&&... ts)
    {
        std::fstream file("temp", std::ios::binary | std::ios::in);
        ASSERT_TRUE(file.is_open());
        Serio::read<ST>({}, file, std::forward<Ts>(ts)...);
    }
    template <typename... Ts>
    void save3(Ts&&... ts)
    {
        Serio::SerializeOptions options;
        options.compactFrom = rand() % 2 == 0;
        options.enableChecksum = rand() % 2 == 0;

#ifdef SERIO_USE_COMPRESSION
        options.compressLevel = (rand() % 2 == 0) ? rand() % 23 : 0;
#endif

#ifdef SERIO_USE_ENCRYPTION
        options.encryptPassword.resize((rand() % 2 == 0) ? rand() % 17 : 0);
        for (auto& c : options.encryptPassword) c = rand() % 128;
        password = options.encryptPassword;
#endif

        Serio::save<ST>(options, "temp", std::forward<Ts>(ts)...);
    }
    template <typename... Ts>
    void load3(Ts&&... ts)
    {
        Serio::DeserializeOptions options;
        options.decryptPassword = password;
        Serio::load<ST>(options, "temp", std::forward<Ts>(ts)...);
    }

    Process()
    {
        Type value1 = Type(), value2 = Type();
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

template <typename T, typename... Ts>
void ProcessAll(Ts&&... ts)
{
    Process<T, Serio::Binary>(std::forward<Ts>(ts)...);
    Process<T, Serio::JSON>(std::forward<Ts>(ts)...);
    Process<T, Serio::XML>(std::forward<Ts>(ts)...);
}

using BasicTypes =
    ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int, long, long long, unsigned char,
                     unsigned short, unsigned int, unsigned long, unsigned long long, float, double>;
using FullTypes =
    ::testing::Types<bool, char, wchar_t, char16_t, char32_t, signed char, short, int, long, long long, unsigned char,
                     unsigned short, unsigned int, unsigned long, unsigned long long, float, double, A, B, D,
                     std::complex<int>, std::complex<float>, std::chrono::steady_clock::time_point, std::bitset<50>,
                     BString<std::string>, BString<std::wstring>
#ifdef SERIO_TEST_BOOST
                     ,
                     boost::container::string, boost::container::wstring
#endif
#ifdef SERIO_TEST_QT
                     ,
                     QString, QByteArray
#endif
                     >;

#define TEST_10(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth0) { ProcessAll<TYPE<TypeParam>>(); }

#define TEST_11(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth1) { ProcessAll<TYPE<TYPE<TypeParam>>>(); }

#define TEST_101(TEST, NAME, TYPE) \
    TEST_10(TEST, NAME, TYPE)      \
    TEST_11(TEST, NAME, TYPE)

#define TEST_20(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth0) { ProcessAll<TYPE<TypeParam, TypeParam>>(); }

#define TEST_21(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth1) { ProcessAll<TYPE<TypeParam, TYPE<TypeParam, TypeParam>>>(); }

#define TEST_201(TEST, NAME, TYPE) \
    TEST_20(TEST, NAME, TYPE)      \
    TEST_21(TEST, NAME, TYPE)

#define TEST_30(TEST, NAME, TYPE) \
    TYPED_TEST(TEST, NAME##Depth0) { ProcessAll<TYPE<TypeParam, TypeParam, TypeParam>>(); }

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
