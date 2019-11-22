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

#ifndef SSERIALIZATION_H
#define SSERIALIZATION_H

#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <forward_list>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <valarray>
#include <vector>

#if __cplusplus >= 201703L
#include <optional>
#include <variant>
#endif

#ifndef SERIO_SIZE
#define SERIO_SIZE 8
#endif

/// @brief Macro for registering custom classes.
///
/// This macro simplifies the process of serializing and deserializing of custom classes. In order
/// to make nearly all of the custom classes usable This macro can be used (there are some rare
/// cases when you have to implement _serialize() and _deserialize() functions yourself.
///
/// Example:
/// @code
/// struct Point
/// {
///     int x, y;
///     SERIO_REGISTER(x, y)
/// };
/// @endcode
#define SERIO_REGISTER(...)                     \
    template <typename Derived>                 \
    friend class Serio::SerializerOps;          \
    template <typename Derived>                 \
    friend class Serio::DeserializerOps;        \
                                                \
    template <typename Serializer>              \
    inline void _serialize(Serializer& C) const \
    {                                           \
        C.process(__VA_ARGS__);                 \
    }                                           \
    template <typename Deserializer>            \
    inline void _deserialize(Deserializer& C)   \
    {                                           \
        C.process(__VA_ARGS__);                 \
    }

/// Main namespace of the library.
namespace Serio
{
#if SERIO_SIZE == 8
/// Data type of the size of containers and such that is serialized and deserialized.
using Size = uint64_t;
#elif SERIO_SIZE == 4
/// Data type of the size of containers and such that is serialized and deserialized.
using Size = uint32_t;
#elif SERIO_SIZE == 2
/// Data type of the size of containers and such that is serialized and deserialized.
using Size = uint16_t;
#else
static_assert(false, "SERIO_SIZE can only be 2, 4 and 8.");
using Size = uint64_t;
#endif

/// Array of 8 bit types.
using ByteArray = std::basic_string<char>;

/// Details of the namespace is in this namespace.
namespace Impl
{
namespace Number
{
template <typename T, size_t N>
struct Number
{
    static inline void serialize(const T& data, char* buffer)
    {
        Number<T, N - 1>::serialize(data, buffer);
        buffer[N] = char(data >> (N * 8));
    }
    static inline void deserialize(T& data, const char* buffer)
    {
        Number<T, N - 1>::deserialize(data, buffer);
        data |= T(uint8_t(buffer[N])) << (N * 8);
    }
};

template <typename T>
struct Number<T, 0>
{
    static inline void serialize(const T& data, char* buffer) { buffer[0] = char(data); }
    static inline void deserialize(T& data, const char* buffer) { data |= T(uint8_t(buffer[0])); }
};

template <typename T>
inline void _serialize(const T& data, char* buffer)
{
    Number<T, sizeof(T) - 1>::serialize(data, buffer);
}
template <typename T>
inline void _deserialize(T& data, const char* buffer)
{
    data = 0;
    Number<T, sizeof(T) - 1>::deserialize(data, buffer);
}

template <typename T>
void serialize(const T& data, char* buffer)
{
    if (sizeof(T) == 1)
        _serialize(*reinterpret_cast<const uint8_t*>(&data), buffer);
    else if (sizeof(T) == 2)
        _serialize(*reinterpret_cast<const uint16_t*>(&data), buffer);
    else if (sizeof(T) == 4)
        _serialize(*reinterpret_cast<const uint32_t*>(&data), buffer);
    else if (sizeof(T) == 8)
        _serialize(*reinterpret_cast<const uint64_t*>(&data), buffer);
#ifdef __SIZEOF_INT128__
    else if (sizeof(T) == 16)
        _serialize(*reinterpret_cast<const __uint128_t*>(&data), buffer);
#endif
    else
        throw std::runtime_error("Basic data type is not supported for serialization");
}
template <typename T>
void deserialize(T& data, const char* buffer)
{
    if (sizeof(T) == 1)
        _deserialize(*reinterpret_cast<uint8_t*>(&data), buffer);
    else if (sizeof(T) == 2)
        _deserialize(*reinterpret_cast<uint16_t*>(&data), buffer);
    else if (sizeof(T) == 4)
        _deserialize(*reinterpret_cast<uint32_t*>(&data), buffer);
    else if (sizeof(T) == 8)
        _deserialize(*reinterpret_cast<uint64_t*>(&data), buffer);
#ifdef __SIZEOF_INT128__
    else if (sizeof(T) == 16)
        _deserialize(*reinterpret_cast<__uint128_t*>(&data), buffer);
#endif
    else
        throw std::runtime_error("Basic data type is not supported for deserialization");
}
}  // namespace Number

namespace Bitset
{
template <size_t N, size_t I, size_t J>
struct Bit
{
    static inline void serialize(const std::bitset<N>& C, char* buffer)
    {
        Bit<N, I, J - 1>::serialize(C, buffer);
        if (I + J < N) buffer[I] |= (C[I * 8 + J] << J) & (1 << J);
    }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        Bit<N, I, J - 1>::deserialize(C, buffer);
        if (I + J < N) C[I * 8 + J] = (buffer[I] >> J) & 1;
    }
};

template <size_t N, size_t I>
struct Bit<N, I, 0>
{
    static inline void serialize(const std::bitset<N>& C, char* buffer) { buffer[I] = C[I * 8]; }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        C[I * 8] = buffer[I] & 1;
    }
};

template <size_t N, size_t I>
struct Char
{
    static inline void serialize(const std::bitset<N>& C, char* buffer)
    {
        Char<N, I - 1>::serialize(C, buffer);
        Bit<N, I, 7>::serialize(C, buffer);
    }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        Char<N, I - 1>::deserialize(C, buffer);
        Bit<N, I, 7>::deserialize(C, buffer);
    }
};

template <size_t N>
struct Char<N, 0>
{
    static inline void serialize(const std::bitset<N>& C, char* buffer)
    {
        Bit<N, 0, 7>::serialize(C, buffer);
    }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        Bit<N, 0, 7>::deserialize(C, buffer);
    }
};

template <size_t N>
inline void serialize(const std::bitset<N>& C, char* buffer)
{
    if (N % 8 == 0)
        Char<N, N / 8 - 1>::serialize(C, buffer);
    else
        Char<N, N / 8>::serialize(C, buffer);
}
template <size_t N>
inline void deserialize(std::bitset<N>& C, const char* buffer)
{
    if (N % 8 == 0)
        Char<N, N / 8 - 1>::deserialize(C, buffer);
    else
        Char<N, N / 8>::deserialize(C, buffer);
}
}  // namespace Bitset

namespace Tuple
{
template <class Tp, size_t N>
struct Tuple
{
    template <typename Serializer>
    static inline void serialize(Serializer& C, const Tp& T)
    {
        Tuple<Tp, N - 1>::serialize(C, T);
        C << std::get<N>(T);
    }
    template <typename Deserializer>
    static inline void deserialize(Deserializer& C, Tp& T)
    {
        Tuple<Tp, N - 1>::deserialize(C, T);
        C >> std::get<N>(T);
    }
};

template <class Tp>
struct Tuple<Tp, 0>
{
    template <typename Serializer>
    static inline void serialize(Serializer& C, const Tp& T)
    {
        C << std::get<0>(T);
    }
    template <typename Deserializer>
    static inline void deserialize(Deserializer& C, Tp& T)
    {
        C >> std::get<0>(T);
    }
};

template <typename Serializer, typename... Ts>
inline void serialize(Serializer& C, const std::tuple<Ts...>& tuple)
{
    Tuple<decltype(tuple), sizeof...(Ts) - 1>::serialize(C, tuple);
}
template <typename Serializer, typename... Ts>
inline void deserialize(Serializer& C, std::tuple<Ts...>& tuple)
{
    Tuple<decltype(tuple), sizeof...(Ts) - 1>::deserialize(C, tuple);
}
}  // namespace Tuple

#if __cplusplus >= 201703L
namespace Variant
{
template <class Tp, size_t N>
struct Variant
{
    template <typename Serializer, typename Vr>
    static inline void serialize(Serializer& C, const Vr& V)
    {
        Variant<Tp, N - 1>::serialize(C, V);
        if (V.index() == N) C << std::get<N>(V);
    }
    template <typename Serializer, typename Vr>
    static inline void deserialize(Serializer& C, Size index, Vr& V)
    {
        Variant<Tp, N - 1>::deserialize(C, index, V);
        if (index == N)
        {
            typename std::tuple_element<N, Tp>::type E;
            C >> E;
            V = std::move(E);
        }
    }
};

template <typename Tp>
struct Variant<Tp, 0>
{
    template <typename Serializer, typename Vr>
    static inline void serialize(Serializer& C, const Vr& V)
    {
        if (V.index() == 0) C << std::get<0>(V);
    }
    template <typename Serializer, typename Vr>
    static inline void deserialize(Serializer& C, Size index, Vr& V)
    {
        if (index == 0)
        {
            typename std::tuple_element<0, Tp>::type E;
            C >> E;
            V = std::move(E);
        }
    }
};

template <typename Serializer, typename... Ts>
inline void serialize(Serializer& C, const std::variant<Ts...>& V)
{
    Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::serialize(C, V);
}
template <typename Serializer, typename... Ts>
inline void deserialize(Serializer& C, Size index, std::variant<Ts...>& V)
{
    Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(C, index, V);
}
}  // namespace Variant
#endif

template <typename Iter>
size_t iteratableSize(const Iter& I)
{
    return I.size();
}
template <typename T, typename Alloc>
size_t iteratableSize(const std::forward_list<T, Alloc>& I)
{
    return size_t(std::distance(I.begin(), I.end()));
}

template <typename T, typename Sequence>
class DerivedQueue : public std::queue<T, Sequence>
{
public:
    using std::queue<T, Sequence>::c;
};
template <typename T, typename Sequence>
class DerivedPQueue : public std::priority_queue<T, Sequence>
{
public:
    using std::priority_queue<T, Sequence>::c;
};
template <typename T, typename Sequence>
class DerivedStack : public std::stack<T, Sequence>
{
public:
    using std::stack<T, Sequence>::c;
};

/// Reads binary data from a file.
///
/// @param path Path of the file to be read
/// @param data Destination of the data that is read from the file
/// @returns True if succeeds otherwise false
inline bool read(const std::string& path, ByteArray& data)
{
    std::basic_ifstream<char> stream(path, std::ios::binary | std::ios::in);
    if (!stream.is_open()) return false;

    stream.seekg(0, std::ios::end);
    auto size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    data.assign(size_t(size), 0);
    auto rsize = stream.rdbuf()->sgetn(&data.front(), std::streamsize(size));
    if (rsize != size) return false;

    stream.close();
    return true;
}

/// Writes a byte array into a file and returns weather it succeeds or not.
///
/// @param path Path of the file to be writen.
/// @param data The data that needs to be written into the file
/// @returns True if succeeds otherwise false
inline bool write(const std::string& path, const ByteArray& data)
{
    std::basic_ofstream<char> stream(path, std::ios::binary | std::ios::out);
    if (!stream.is_open()) return false;

    auto dsize = std::streamsize(data.size());
    auto size = stream.rdbuf()->sputn(data.data(), dsize);
    if (size != dsize) return false;

    stream.flush();
    stream.close();
    return true;
}
}  // namespace Impl

///@brief Wrapper for raw array.
///
/// This class wraps the raw arrays so they can be serialized and deserialized too.
///
/// Example:
/// @code
/// int A[3] = {1, 2, 3};
/// auto result = Serio::serialize(Serio::Array<int>(3, A));
/// @endcode
///
/// Another example:
/// @code
/// struct A
/// {
///     int B[3];
///     SERIO_REGISTER(Serio::Array<int>(3, B))
/// };
/// @endcode
template <typename T>
struct Array
{
    size_t size;
    T* data;
    inline Array(const T* data, size_t size) : size(size), data(const_cast<T*>(data)) {}
};

/// @brief Processes higher structures for serialization and size calculation.
///
/// This class breaks higher structurs into simple elements and passes them on to the class that
/// handles them to calculate size of data or serialize it.
template <typename Derived>
class SerializerOps
{
    inline Derived& This() { return *static_cast<Derived*>(this); }

    template <typename Iter>
    inline Derived& iteratable(const Iter& C)
    {
        This() << Size(Impl::iteratableSize(C));
        for (const auto& S : C) This() << S;
        return This();
    }

public:
    template <typename T>
    inline typename std::enable_if<std::is_class<T>::value, Derived&>::type operator<<(const T& C)
    {
        C._serialize(This());
        return This();
    }

    template <typename T, typename Traits, typename Alloc>
    inline Derived& operator<<(const std::basic_string<T, Traits, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, typename Alloc>
    inline Derived& operator<<(const std::vector<T, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename Alloc>
    inline Derived& operator<<(const std::vector<bool, Alloc>& C)
    {
        This() << Size(Impl::iteratableSize(C));
        for (bool S : C) This() << S;
        return This();
    }

    template <typename T, typename Alloc>
    inline Derived& operator<<(const std::deque<T, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, typename Alloc>
    inline Derived& operator<<(const std::list<T, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, typename Alloc>
    inline Derived& operator<<(const std::forward_list<T, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, size_t N>
    inline Derived& operator<<(const std::array<T, N>& C)
    {
        for (const auto& S : C) This() << S;
        return This();
    }

    template <typename T>
    inline Derived& operator<<(const std::valarray<T>& C)
    {
        return iteratable(C);
    }

    template <typename T>
    inline Derived& operator<<(const Array<T>& C)
    {
        This() << Size(C.size);
        for (size_t i = 0; i < C.size; ++i) This() << C.data[i];
        return This();
    }

    template <typename T, typename Comp, typename Alloc>
    inline Derived& operator<<(const std::set<T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator<<(const std::unordered_set<T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, typename Comp, typename Alloc>
    inline Derived& operator<<(const std::multiset<T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator<<(const std::unordered_multiset<T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename K, typename T, typename Comp, typename Alloc>
    inline Derived& operator<<(const std::map<K, T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator<<(const std::unordered_map<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename K, typename T, typename Comp, typename Alloc>
    inline Derived& operator<<(const std::multimap<K, T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator<<(const std::unordered_multimap<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T, typename Sequence>
    inline Derived& operator<<(const std::queue<T, Sequence>& C)
    {
        const auto& D = reinterpret_cast<const Impl::DerivedQueue<T, Sequence>&>(C);
        return This() << D.c;
    }

    template <typename T, typename Sequence>
    inline Derived& operator<<(const std::priority_queue<T, Sequence>& C)
    {
        const auto& D = reinterpret_cast<const Impl::DerivedPQueue<T, Sequence>&>(C);
        return This() << D.c;
    }

    template <typename T, typename Sequence>
    inline Derived& operator<<(const std::stack<T, Sequence>& C)
    {
        const auto& D = reinterpret_cast<const Impl::DerivedStack<T, Sequence>&>(C);
        return This() << D.c;
    }

    template <typename T1, typename T2>
    inline Derived& operator<<(const std::pair<T1, T2>& C)
    {
        return This() << C.first << C.second;
    }

    template <typename... Ts>
    inline Derived& operator<<(const std::tuple<Ts...>& C)
    {
        Impl::Tuple::serialize(This(), C);
        return This();
    }

    template <typename T>
    inline Derived& operator<<(const std::shared_ptr<T>& C)
    {
        This() << bool(C);
        if (C) This() << *C.get();
        return This();
    }

    template <typename T>
    inline Derived& operator<<(const std::unique_ptr<T>& C)
    {
        This() << bool(C);
        if (C) This() << *C.get();
        return This();
    }

    template <typename T>
    inline Derived& operator<<(const std::complex<T>& C)
    {
        return This() << C.real() << C.imag();
    }

    template <typename Clock, typename Dur>
    inline Derived& operator<<(const std::chrono::time_point<Clock, Dur>& C)
    {
        using rep = typename std::chrono::time_point<Clock, Dur>::rep;
        return This() << reinterpret_cast<const rep&>(C);
    }

#if __cplusplus >= 201703L
    template <typename T>
    inline Derived& operator<<(const std::optional<T>& C)
    {
        This() << C.has_value();
        if (C.has_value()) This() << C.value();
        return This();
    }

    template <typename... Ts>
    Derived& operator<<(const std::variant<Ts...>& C)
    {
        if (C.index() == std::variant_npos)
            This() << Size(-1);
        else
        {
            This() << Size(C.index());
            Impl::Variant::serialize(This(), C);
        }

        return This();
    }
#endif

    inline Derived& process() { return This(); }

    template <typename Head, typename... Tail>
    inline Derived& process(const Head& head, Tail&&... tail)
    {
        This() << head;
        return process(std::forward<Tail>(tail)...);
    }
};

/// @brief Processes higher structures for deserialization.
///
/// This class breaks higher structurs into simple elements and passes them on to the class that
/// handles them to deserialize the data.
template <typename Derived>
class DeserializerOps
{
    inline Derived& This() { return *static_cast<Derived*>(this); }

    template <typename Iter, typename T>
    Derived& assignable(Iter& C)
    {
        Size size;
        This() >> size;

        C.resize(size);
        for (auto& value : C) This() >> value;
        return This();
    }

    template <typename Iter, typename T>
    Derived& iteratable(Iter& C)
    {
        C.clear();
        Size size;
        This() >> size;

        auto it = C.begin();
        for (Size i = 0; i < size; ++i)
        {
            T value;
            This() >> value;
            it = C.emplace_hint(it, std::move(value));
        }

        return This();
    }

    template <typename Ptr, typename T>
    inline Derived& pointer(Ptr& C)
    {
        bool has;
        This() >> has;

        if (has)
        {
            auto* value = new T();
            This() >> *value;
            C.reset(value);
        }
        else
            C.reset();

        return This();
    }

public:
    template <typename T>
    inline typename std::enable_if<std::is_class<T>::value, Derived&>::type operator>>(T& C)
    {
        C._deserialize(This());
        return This();
    }

    template <typename T, typename Traits, typename Alloc>
    inline Derived& operator>>(std::basic_string<T, Traits, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }

    template <typename T, typename Alloc>
    inline Derived& operator>>(std::vector<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }

    template <typename Alloc>
    Derived& operator>>(std::vector<bool, Alloc>& C)
    {
        Size size;
        This() >> size;
        C.resize(size);

        for (Size i = 0; i < size; ++i)
        {
            bool value;
            This() >> value;
            C[i] = value;
        }

        return This();
    }

    template <typename T, typename Alloc>
    inline Derived& operator>>(std::deque<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }

    template <typename T, typename Alloc>
    inline Derived& operator>>(std::list<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }

    template <typename T, typename Alloc>
    inline Derived& operator>>(std::forward_list<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }

    template <typename T, size_t N>
    inline Derived& operator>>(std::array<T, N>& C)
    {
        for (Size i = 0; i < N; ++i) This() >> C[i];
        return This();
    }

    template <typename T>
    inline Derived& operator>>(std::valarray<T>& C)
    {
        return assignable<decltype(C), T>(C);
    }

    template <typename T>
    inline Derived& operator>>(Array<T> C)
    {
        Size size;
        This() >> size;
        for (size_t i = 0; i < size; ++i) This() >> C.data[i];
        return This();
    }

    template <typename T, typename Comp, typename Alloc>
    inline Derived& operator>>(std::set<T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }

    template <typename T, typename Comp, typename Alloc>
    inline Derived& operator>>(std::multiset<T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }

    template <typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator>>(std::unordered_set<T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }

    template <typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator>>(std::unordered_multiset<T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }

    template <typename K, typename T, typename Comp, typename Alloc>
    inline Derived& operator>>(std::map<K, T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }

    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator>>(std::unordered_map<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }

    template <typename K, typename T, typename Comp, typename Alloc>
    inline Derived& operator>>(std::multimap<K, T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }

    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    inline Derived& operator>>(std::unordered_multimap<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }

    template <typename T, typename Sequence>
    inline Derived& operator>>(std::queue<T, Sequence>& C)
    {
        auto& D = reinterpret_cast<Impl::DerivedQueue<T, Sequence>&>(C);
        return This() >> D.c;
    }

    template <typename T, typename Sequence>
    inline Derived& operator>>(std::priority_queue<T, Sequence>& C)
    {
        auto& D = reinterpret_cast<Impl::DerivedPQueue<T, Sequence>&>(C);
        return This() >> D.c;
    }

    template <typename T, typename Sequence>
    inline Derived& operator>>(std::stack<T, Sequence>& C)
    {
        auto& D = reinterpret_cast<Impl::DerivedStack<T, Sequence>&>(C);
        return This() >> D.c;
    }

    template <typename T1, typename T2>
    inline Derived& operator>>(std::pair<T1, T2>& C)
    {
        return This() >> C.first >> C.second;
    }

    template <typename... Ts>
    inline Derived& operator>>(std::tuple<Ts...>& C)
    {
        Impl::Tuple::deserialize(This(), C);
        return This();
    }

    template <typename T>
    inline Derived& operator>>(std::shared_ptr<T>& C)
    {
        return pointer<decltype(C), T>(C);
    }

    template <typename T, typename Deleter>
    inline Derived& operator>>(std::unique_ptr<T, Deleter>& C)
    {
        return pointer<decltype(C), T>(C);
    }

    template <typename T>
    inline Derived& operator>>(std::complex<T>& C)
    {
        T real, imag;
        This() >> real >> imag;
        C = std::complex<T>(real, imag);
        return This();
    }

    template <typename Clock, typename Dur>
    inline Derived& operator>>(std::chrono::time_point<Clock, Dur>& C)
    {
        using rep = typename std::chrono::time_point<Clock, Dur>::rep;
        return This() >> reinterpret_cast<rep&>(C);
    }

#if __cplusplus >= 201703L
    template <typename T>
    Derived& operator>>(std::optional<T>& C)
    {
        bool has;
        This() >> has;

        if (has)
        {
            T value;
            This() >> value;
            C.emplace(std::move(value));
        }
        else
            C.reset();

        return This();
    }

    template <typename... Ts>
    Derived& operator>>(std::variant<Ts...>& C)
    {
        Size index;
        This() >> index;

        if (index == Size(-1))
            ;  // TODO reset C somehow
        else
            Impl::Variant::deserialize(This(), index, C);

        return This();
    }
#endif

    inline Derived& process() { return This(); }

    template <typename Head, typename... Tail>
    inline Derived& process(Head& head, Tail&&... tail)
    {
        This() >> head;
        return process(std::forward<Tail>(tail)...);
    }
};

/// @brief This class implements the size calculation of basic data types.
template <typename Derived>
class CalculatorBase
{
    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

public:
    size_t size;

    inline CalculatorBase(size_t size = 0) : size(size) {}

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value,
                                   Derived&>::type
    operator<<(const T& C)
    {
        size += sizeof(C);
        return This();
    }

    template <typename Traits, typename Alloc>
    inline Derived& operator<<(const std::basic_string<char, Traits, Alloc>& C)
    {
        size += sizeof(Size) + C.size();
        return This();
    }

    template <size_t N>
    inline Derived& operator<<(const std::bitset<N>& C)
    {
        (void)C;
        size += ceil(N / 8.0);
        return This();
    }
};

/// @brief This class implements the serialization of basic data types.
template <typename Derived>
struct SerializerBase
{
    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

public:
    char* buffer;

    inline SerializerBase(char* buffer = nullptr) : buffer(buffer) {}

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value,
                                   Derived&>::type
    operator<<(const T& C)
    {
        Impl::Number::serialize(C, buffer);
        buffer += sizeof(C);
        return This();
    }

    template <typename Traits, typename Alloc>
    inline Derived& operator<<(const std::basic_string<char, Traits, Alloc>& C)
    {
        This() << Size(C.size());
        std::memcpy(buffer, C.data(), C.size());
        buffer += C.size();
        return This();
    }

    template <size_t N>
    inline Derived& operator<<(const std::bitset<N>& C)
    {
        Impl::Bitset::serialize(C, buffer);
        buffer += size_t(std::ceil(N / 8.0));
        return This();
    }
};

/// @brief This class implements the deserialization of basic data types.
template <typename Derived>
struct DeserializerBase
{
    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

public:
    const char* buffer;

    inline DeserializerBase(const char* buffer = nullptr) : buffer(buffer) {}

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value,
                                   Derived&>::type
    operator>>(T& C)
    {
        Impl::Number::deserialize(C, buffer);
        buffer += sizeof(C);
        return This();
    }

    template <typename Traits, typename Alloc>
    Derived& operator>>(std::basic_string<char, Traits, Alloc>& C)
    {
        Size size;
        This() >> size;
        C.resize(size);
        std::memcpy(&C.front(), buffer, size);
        buffer += size;
        return This();
    }

    template <size_t N>
    inline Derived& operator>>(std::bitset<N>& C)
    {
        Impl::Bitset::deserialize(C, buffer);
        buffer += size_t(std::ceil(N / 8.0));
        return This();
    }
};

template <typename Derived>
struct StreamSerializerBase
{
    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

public:
    std::basic_ostream<char>* stream;

    inline StreamSerializerBase(std::basic_ostream<char>* stream) : stream(stream) {}

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value,
                                   Derived&>::type
    operator<<(const T& C)
    {
        char buffer[sizeof(C)];
        Impl::Number::serialize(C, buffer);
        stream->rdbuf()->sputn(buffer, sizeof(C));
        return This();
    }

    template <typename Traits, typename Alloc>
    inline Derived& operator<<(const std::basic_string<char, Traits, Alloc>& C)
    {
        stream->rdbuf()->sputn(C.data(), C.size());
        return This();
    }

    template <size_t N>
    inline Derived& operator<<(const std::bitset<N>& C)
    {
        size_t size = std::ceil(N / 8.0);
        ByteArray buffer(size, 0);
        Impl::Bitset::serialize(C, &buffer.front());
        stream->rdbuf()->sputn(buffer.data(), buffer.size());
        return This();
    }
};

template <typename Derived>
struct StreamDeserializerBase
{
    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

public:
    std::basic_istream<char>* stream;

    inline StreamDeserializerBase(std::basic_istream<char>* stream = nullptr) : stream(stream) {}

    void fill() {}

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value,
                                   Derived&>::type
    operator>>(T& C)
    {
        ByteArray buffer(sizeof(C), 0);
        stream->rdbuf()->sgetn(&buffer.front(), sizeof(C));
        Impl::Number::deserialize(C, buffer.data());
        return This();
    }

    template <typename Traits, typename Alloc>
    Derived& operator>>(std::basic_string<char, Traits, Alloc>& C)
    {
        Size size;
        This() >> size;
        C.resize(size);
        stream->rdbuf()->sputn(&C.front(), size);
        return This();
    }

    template <size_t N>
    inline Derived& operator>>(std::bitset<N>& C)
    {
        size_t size = std::ceil(N / 8.0);
        ByteArray buffer(size, 0);
        stream->rdbuf()->sgetn(&buffer.front(), buffer.size());
        Impl::Bitset::deserialize(C, buffer.data());
        return This();
    }
};

/// @brief This class calculates size of any supported types.
struct Calculator : CalculatorBase<Calculator>, SerializerOps<Calculator>
{
    using Base = CalculatorBase<Calculator>;
    using Ops = SerializerOps<Calculator>;
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};

/// @brief This class serializes any of the supported types.
struct Serializer : SerializerBase<Serializer>, SerializerOps<Serializer>
{
    using Base = SerializerBase<Serializer>;
    using Ops = SerializerOps<Serializer>;
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};

/// @brief This class deserializes any of the supported types.
struct Deserializer : DeserializerBase<Deserializer>, DeserializerOps<Deserializer>
{
    using Base = DeserializerBase<Deserializer>;
    using Ops = DeserializerOps<Deserializer>;
    using Base::Base;
    using Ops::operator>>;
    using Base::operator>>;
};

struct StreamSerializer : StreamSerializerBase<StreamSerializer>, SerializerOps<StreamSerializer>
{
    using Base = StreamSerializerBase<StreamSerializer>;
    using Ops = SerializerOps<StreamSerializer>;
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};
struct StreamDeserializer : StreamDeserializerBase<StreamDeserializer>,
                            DeserializerOps<StreamDeserializer>
{
    using Base = StreamDeserializerBase<StreamDeserializer>;
    using Ops = DeserializerOps<StreamDeserializer>;
    using Base::Base;
    using Ops::operator>>;
    using Base::operator>>;
};

/// Calculates the size of unlimited number of input arguments (serializable data types).
///
/// Example:
/// @code
/// int A, B;
/// std::vector<int> C = {1, 2, 3, 4};
/// auto size = Serio::size(A, B, C);
/// @endcode
///
/// @param head First parameter to be serialized
/// @param tail Rest of the parameters to be serialized
/// @returns number of bytes that the input arguments will consume
/// @see deserialize()
template <typename Head, typename... Tail>
inline size_t size(const Head& head, Tail&&... tail)
{
    return Calculator().process(head, std::forward<Tail>(tail)...).size;
}

/// Serializes and unlimited number of input arguments (serializable data types).
///
/// Example:
/// @code
/// int A, B;
/// std::vector<int> C = {1, 2, 3, 4};
/// char data[Serio::size(A,B,C)];
/// Serio::fill(data, A, B, C);
/// @endcode
///
/// @param data Destination of serialized data
/// @param head First parameter to be serialized
/// @param tail Rest of the parameters to be serialized
/// @returns The number of bytes written to sequence
/// @see serialize()
template <typename Head, typename... Tail>
inline size_t fill(char* data, const Head& head, Tail&&... tail)
{
    return size_t(Serializer(data).process(head, std::forward<Tail>(tail)...).buffer - data);
}

/// Deserializes and unlimited number of input arguments (deserializable data types).
///
/// Example:
/// @code
/// int A, B;
/// std::vector<int> C;
/// const char *data; // Must contain the serialized data
/// auto size = Serio::deserialize(data, A, B, C);
/// @endcode
///
/// @param data Sequence of bytes containing serialized data
/// @param head First parameter to be deserialized
/// @param tail Rest of the parameters to be deserialized
/// @returns The number of bytes consumed from char sequence
/// @see serialize(), deserialize()
template <typename Head, typename... Tail>
inline size_t deserialize(const char* data, Head& head, Tail&&... tail)
{
    return size_t(Deserializer(data).process(head, std::forward<Tail>(tail)...).buffer - data);
}

/// Serializes and unlimited number of input arguments (serializable data types).
///
/// Example:
/// @code
/// int A, B;
/// std::vector<int> C = {1, 2, 3, 4};
/// auto data = Serio::serialize(A, B, C);
/// @endcode
///
/// @param head First parameter to be serialized
/// @param tail Rest of the parameters to be serialized
/// @returns Array of bytes containing the serialized data
/// @see deserialize()
template <typename Head, typename... Tail>
inline ByteArray serialize(const Head& head, Tail&&... tail)
{
    ByteArray data(Serio::size(head, std::forward<Tail>(tail)...), 0);
    Serio::fill(&data.front(), head, std::forward<Tail>(tail)...);
    return data;
}

/// Deserializes and unlimited number of input arguments (deserializable data types).
///
/// Example:
/// @code
/// int A, B;
/// std::vector<int> C;
/// Serio::ByteArray data; // Must contain the serialized data
/// auto size = Serio::deserialize(data, A, B, C);
/// @endcode
///
/// @param data Array of bytes containing serialized data
/// @param head First parameter to be deserialized
/// @param tail Rest of the parameters to be deserialized
/// @returns The number of bytes consumed from byte array
/// @see serialize(), deserialize()
template <typename Head, typename... Tail>
inline size_t deserialize(const ByteArray& data, Head& head, Tail&&... tail)
{
    return deserialize(&data.front(), head, std::forward<Tail>(tail)...);
}

/// Serializes and unlimited number of input arguments (serializable data types) and writes it to a
/// file.
///
/// Example:
/// @code
/// int A, B;
/// std::vector<int> C = {1, 2, 3, 4};
/// assert(Serio::save("file", A, B, C));
/// @endcode
///
/// @param path Path of the file to be writen.
/// @param head First parameter to be serialized
/// @param tail Rest of the parameters to be serialized
/// @returns True if succeeds otherwise false
/// @see serialize()
template <typename Head, typename... Tail>
inline bool save(const std::string& path, const Head& head, Tail&&... tail)
{
    return Impl::write(path, serialize(head, std::forward<Tail>(tail)...));
}

/// Reads data from a file and deserializes and unlimited number of input arguments (deserializable
/// data types).
///
/// Example:
/// @code
/// int A, B;
/// std::vector<int> C;
/// Serio::ByteArray data; // Must contain the serialized data
/// assert(Serio::load("file", A, B, C));
/// @endcode
///
/// @param path Path of the file to be read
/// @param head First parameter to be deserialized
/// @param tail Rest of the parameters to be deserialized
/// @returns True if succeeds otherwise false
/// @see deserialize()
template <typename Head, typename... Tail>
inline bool load(const std::string& path, Head& head, Tail&&... tail)
{
    ByteArray A;
    if (!Impl::read(path, A)) return false;
    return deserialize(A, head, std::forward<Tail>(tail)...) == A.size();
}

template <typename Head, typename... Tail>
inline void streamSerialize(std::basic_ostream<char>* stream, const Head& head, Tail&&... tail)
{
    StreamSerializer serializer(stream);
    serializer.process(head, std::forward<Tail>(tail)...);
}
template <typename Head, typename... Tail>
inline void streamDeserialize(std::basic_istream<char>* stream, Head& head, Tail&&... tail)
{
    StreamDeserializer deserializer(stream);
    deserializer.process(head, std::forward<Tail>(tail)...);
}
}  // namespace Serio

#endif  // SSERIALIZATION_H
