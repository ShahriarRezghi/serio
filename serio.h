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

#ifndef SSERIALIZATION_H
#define SSERIALIZATION_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <array>
#include <bitset>
#include <chrono>
#include <complex>
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
#endif

#ifndef SERIO_SIZE
#define SERIO_SIZE 8
#endif

#define SERIO_REGISTER_CLASS(...)                                  \
    template <typename Derived>                                    \
    friend struct Serio::SerializerOps;                            \
    template <typename Derived>                                    \
    friend struct Serio::DeserializerOps;                          \
                                                                   \
    template <typename Derived>                                    \
    inline void _serialize(Serio::SerializerOps<Derived>* C) const \
    {                                                              \
        C->process(__VA_ARGS__);                                   \
    }                                                              \
    template <typename Derived>                                    \
    inline void _deserialize(Serio::DeserializerOps<Derived>* C)   \
    {                                                              \
        C->process(__VA_ARGS__);                                   \
    }

#define SERIALIZER_CREATE_TYPE(TYPE) \
    inline Derived& operator<<(const TYPE& C) { return dataType(C); }

#define DESERIALIZER_CREATE_TYPE(TYPE) \
    inline Derived& operator>>(TYPE& C) { return dataType(C); }

#define FUNC_FOR_TYPES(FUNC) \
    FUNC(char)               \
    FUNC(signed char)        \
    FUNC(short)              \
    FUNC(int)                \
    FUNC(long)               \
    FUNC(long long)          \
    FUNC(unsigned char)      \
    FUNC(unsigned short)     \
    FUNC(unsigned int)       \
    FUNC(unsigned long)      \
    FUNC(unsigned long long) \
    FUNC(wchar_t)            \
    FUNC(bool)               \
    FUNC(float)              \
    FUNC(double)             \
    FUNC(long double)

namespace Serio
{
#if SERIO_SIZE == 8
using Size = uint64_t;
#elif SERIO_SIZE == 4
using Size = uint32_t;
#elif SERIO_SIZE == 2
using Size = uint16_t;
#endif

namespace Impl
{
namespace Number
{
template <typename T, size_t N>
struct NumberRecurse
{
    static inline void serialize(const T& data, char* buffer)
    {
        NumberRecurse<T, N - 1>::serialize(data, buffer);
        buffer[N] = char(data >> (N * 8));
    }
    static inline void deserialize(T& data, const char* buffer)
    {
        NumberRecurse<T, N - 1>::deserialize(data, buffer);
        data |= T(uint8_t(buffer[N])) << (N * 8);
    }
};

template <typename T>
struct NumberRecurse<T, 0>
{
    static inline void serialize(const T& data, char* buffer) { buffer[0] = char(data); }
    static inline void deserialize(T& data, const char* buffer) { data |= T(uint8_t(buffer[0])); }
};

template <typename T>
inline void _serialize(const T& data, char* buffer)
{
    NumberRecurse<T, sizeof(T) - 1>::serialize(data, buffer);
}

template <typename T>
inline void _deserialize(T& data, const char* buffer)
{
    data = 0;
    NumberRecurse<T, sizeof(T) - 1>::deserialize(data, buffer);
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
struct BitsetBitRecurse
{
    static inline void serialize(const std::bitset<N>& C, char* buffer)
    {
        BitsetBitRecurse<N, I, J - 1>::serialize(C, buffer);
        if (I + J < N) buffer[I] |= (C[I * 8 + J] << J) & (1 << J);
    }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        BitsetBitRecurse<N, I, J - 1>::deserialize(C, buffer);
        if (I + J < N) C[I * 8 + J] = (buffer[I] >> J) & 1;
    }
};

template <size_t N, size_t I>
struct BitsetBitRecurse<N, I, 0>
{
    static inline void serialize(const std::bitset<N>& C, char* buffer) { buffer[I] = C[I * 8]; }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        C[I * 8] = buffer[I] & 1;
    }
};

template <size_t N, size_t I>
struct BitsetCharRecurse
{
    static inline void serialize(const std::bitset<N>& C, char* buffer)
    {
        BitsetCharRecurse<N, I - 1>::serialize(C, buffer);
        BitsetBitRecurse<N, I, 7>::serialize(C, buffer);
    }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        BitsetCharRecurse<N, I - 1>::deserialize(C, buffer);
        BitsetBitRecurse<N, I, 7>::deserialize(C, buffer);
    }
};

template <size_t N>
struct BitsetCharRecurse<N, 0>
{
    static inline void serialize(const std::bitset<N>& C, char* buffer)
    {
        BitsetBitRecurse<N, 0, 7>::serialize(C, buffer);
    }
    static inline void deserialize(std::bitset<N>& C, const char* buffer)
    {
        BitsetBitRecurse<N, 0, 7>::deserialize(C, buffer);
    }
};

template <size_t N>
inline void serialize(const std::bitset<N>& C, char* buffer)
{
    if (N % 8 == 0)
        BitsetCharRecurse<N, N / 8 - 1>::serialize(C, buffer);
    else
        BitsetCharRecurse<N, N / 8>::serialize(C, buffer);
}

template <size_t N>
inline void deserialize(std::bitset<N>& C, const char* buffer)
{
    if (N % 8 == 0)
        BitsetCharRecurse<N, N / 8 - 1>::deserialize(C, buffer);
    else
        BitsetCharRecurse<N, N / 8>::deserialize(C, buffer);
}
}  // namespace Bitset

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

template <class Tuple, size_t N>
struct TupleRecurse
{
    template <typename A>
    static inline void serialize(A& C, const Tuple& T)
    {
        TupleRecurse<Tuple, N - 1>::serialize(C, T);
        C << std::get<N - 1>(T);
    }
    template <typename A>
    static inline void deserialize(A& C, Tuple& T)
    {
        TupleRecurse<Tuple, N - 1>::deserialize(C, T);
        C >> std::get<N - 1>(T);
    }
};

template <class Tuple>
struct TupleRecurse<Tuple, 1>
{
    template <typename A>
    static inline void serialize(A& C, const Tuple& T)
    {
        C << std::get<0>(T);
    }
    template <typename A>
    static inline void deserialize(A& C, Tuple& T)
    {
        C >> std::get<0>(T);
    }
};
}  // namespace Impl

/// This class breaks higher structurs into simple elements and passes them on to the class that
/// handles them
template <typename Derived>
struct SerializerOps
{
    using Ops = SerializerOps;

    inline Derived& This() { return *static_cast<Derived*>(this); }

    template <typename Iter>
    inline Derived& iteratable(const Iter& C)
    {
        auto& A = This();
        A << Size(Impl::iteratableSize(C));
        for (const auto& S : C) A << S;
        return A;
    }

    template <typename T>
    inline Derived operator<<(const T& C)
    {
        C._serialize(this);
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
        return iteratable(C);
    }

    template <typename T>
    inline Derived& operator<<(const std::valarray<T>& C)
    {
        return iteratable(C);
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
        const auto* D = reinterpret_cast<const Impl::DerivedQueue<T, Sequence>*>(&C);
        return iteratable(D->c);
    }

    template <typename T, typename Sequence>
    inline Derived& operator<<(const std::priority_queue<T, Sequence>& C)
    {
        const auto* D = reinterpret_cast<const Impl::DerivedPQueue<T, Sequence>*>(&C);
        return iteratable(D->c);
    }

    template <typename T, typename Sequence>
    inline Derived& operator<<(const std::stack<T, Sequence>& C)
    {
        const auto* D = reinterpret_cast<const Impl::DerivedStack<T, Sequence>*>(&C);
        return iteratable(D->c);
    }

    template <typename T1, typename T2>
    inline Derived& operator<<(const std::pair<T1, T2>& C)
    {
        return This() << C.first << C.second;
    }

    template <typename... Ts>
    inline Derived& operator<<(const std::tuple<Ts...>& C)
    {
        auto& A = This();
        Impl::TupleRecurse<decltype(C), sizeof...(Ts)>::serialize(A, C);
        return A;
    }

    template <typename T>
    inline Derived& operator<<(const std::shared_ptr<T>& C)
    {
        return This() << *C.get();
    }

    template <typename T>
    inline Derived& operator<<(const std::unique_ptr<T>& C)
    {
        return This() << *C.get();
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
        return This() << *reinterpret_cast<rep*>(&C);
    }

#if __cplusplus >= 201703L
    template <typename T>
    inline Derived& operator<<(const std::optional<T>& C)
    {
        auto& A = This();
        A << C.has_value();
        if (C.has_value()) A << C.value();
        return A;
    }
#endif

    inline void process() {}

    template <typename Head, typename... Tail>
    inline void process(const Head& head, Tail&&... tail)
    {
        This() << head;
        process(std::forward<Tail>(tail)...);
    }
};

/// This class breaks higher structurs into simple elements and passes them on to the class that
/// handles them
template <typename Derived>
struct DeserializerOps
{
    using Ops = DeserializerOps;

    inline Derived& This() { return *static_cast<Derived*>(this); }

    template <typename Iter, typename T>
    Derived& assignable(Iter& C)
    {
        auto& A = This();
        Size size;
        A >> size;

        C.resize(size);
        for (auto& value : C) A >> value;
        return A;
    }

    template <typename Iter, typename T>
    Derived& iteratable(Iter& C)
    {
        auto& A = This();
        C.clear();
        Size size;
        A >> size;

        auto it = C.begin();
        for (Size i = 0; i < size; ++i)
        {
            T value;
            A >> value;
            it = C.emplace_hint(it, std::move(value));
        }

        return A;
    }

    template <typename Ptr, typename T>
    inline Derived& pointer(const Ptr& C)
    {
        auto& A = This();
        T* value = new T();
        A >> *value;
        C = value;
        return A;
    }

    template <typename T>
    inline Derived& operator>>(T& C)
    {
        C._deserialize(this);
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
    inline Derived& operator>>(std::vector<bool, Alloc>& C)
    {
        auto& A = This();
        Size size;
        A >> size;
        C.resize(size);

        for (Size i = 0; i < size; ++i)
        {
            bool value;
            A >> value;
            C[i] = value;
        }

        return A;
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
        auto& A = This();
        Size size;
        A >> size;
        for (Size i = 0; i < size; ++i) (*this) >> C[i];
        return A;
    }

    template <typename T>
    inline Derived& operator>>(std::valarray<T>& C)
    {
        return assignable<decltype(C), T>(C);
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
        auto* D = reinterpret_cast<Impl::DerivedQueue<T, Sequence>*>(&C);
        return assignable(D->c);
    }

    template <typename T, typename Sequence>
    inline Derived& operator>>(std::priority_queue<T, Sequence>& C)
    {
        auto* D = reinterpret_cast<Impl::DerivedPQueue<T, Sequence>*>(&C);
        return assignable(D->c);
    }

    template <typename T, typename Sequence>
    inline Derived& operator>>(std::stack<T, Sequence>& C)
    {
        auto* D = reinterpret_cast<Impl::DerivedStack<T, Sequence>*>(&C);
        return assignable(D->c);
    }

    template <typename T1, typename T2>
    inline Derived& operator>>(std::pair<T1, T2>& C)
    {
        return This() >> C.first >> C.second;
    }

    template <typename... Ts>
    inline Derived& operator>>(std::tuple<Ts...>& C)
    {
        auto& A = This();
        Impl::TupleRecurse<decltype(C), sizeof...(Ts)>::deserialize(A, C);
        return A;
    }

    template <typename T>
    inline Derived& operator>>(const std::shared_ptr<T>& C)
    {
        return pointer<decltype(C), T>(C);
    }

    template <typename T>
    inline Derived& operator>>(const std::unique_ptr<T>& C)
    {
        return pointer<decltype(C), T>(C);
    }

    template <typename T>
    inline Derived& operator>>(std::complex<T>& C)
    {
        auto& A = This();
        T real, imag;
        A >> real >> imag;
        C = std::complex<T>(real, imag);
        return A;
    }

    template <typename Clock, typename Dur>
    inline Derived& operator>>(const std::chrono::time_point<Clock, Dur>& C)
    {
        using rep = typename std::chrono::time_point<Clock, Dur>::rep;
        return This() >> *reinterpret_cast<rep*>(&C);
    }

#if __cplusplus >= 201703L
    template <typename T>
    inline Derived& operator>>(std::optional<T>& C)
    {
        auto& A = This();
        bool has;
        A >> has;

        if (has)
        {
            T value;
            A >> value;
            C.emplace(std::move(value));
        }
        else
            C.reset();

        return A;
    }
#endif

    inline void process() {}

    template <typename Head, typename... Tail>
    inline void process(Head& head, Tail&&... tail)
    {
        This() >> head;
        process(std::forward<Tail>(tail)...);
    }
};

/// This class implements size calculation of basic types
template <typename Derived>
struct CalculatorBase
{
    using Base = CalculatorBase;
    size_t size{0};

    CalculatorBase() {}
    CalculatorBase(size_t size) { this->size = size; }

    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

    template <typename T>
    inline Derived& dataType(const T& C)
    {
        size += sizeof(C);
        return This();
    }

    FUNC_FOR_TYPES(SERIALIZER_CREATE_TYPE)

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

/// This class implements serialization of basic types
template <typename Derived>
struct SerializerBase
{
    using Base = SerializerBase;
    char* buffer{nullptr};

    SerializerBase() {}
    SerializerBase(char* buffer) { this->buffer = buffer; }

    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

    template <typename T>
    inline Derived& dataType(const T& C)
    {
        Impl::Number::serialize(C, buffer);
        buffer += sizeof(C);
        return This();
    }

    FUNC_FOR_TYPES(SERIALIZER_CREATE_TYPE)

    template <typename Traits, typename Alloc>
    inline Derived& operator<<(const std::basic_string<char, Traits, Alloc>& C)
    {
        auto& A = This();
        A << Size(C.size());
        std::memcpy(buffer, C.data(), C.size());
        buffer += C.size();
        return A;
    }

    template <size_t N>
    inline Derived& operator<<(const std::bitset<N>& C)
    {
        Impl::Bitset::serialize(C, buffer);
        buffer += size_t(std::ceil(N / 8.0));

        return This();
    }
};

/// This class implements deserialization of basic types
template <typename Derived>
struct DeserializerBase
{
    using Base = DeserializerBase;
    const char* buffer{nullptr};

    DeserializerBase() {}
    DeserializerBase(const char* buffer) { this->buffer = buffer; }

    inline Derived& This() { return *reinterpret_cast<Derived*>(this); }

    template <typename T>
    inline Derived& dataType(T& C)
    {
        Impl::Number::deserialize(C, buffer);
        buffer += sizeof(C);
        return This();
    }

    FUNC_FOR_TYPES(DESERIALIZER_CREATE_TYPE)

    template <typename Traits, typename Alloc>
    inline Derived& operator>>(std::basic_string<char, Traits, Alloc>& C)
    {
        auto& A = This();
        Size size;
        A >> size;
        C.resize(size);
        std::memcpy(&C.front(), buffer, size);
        buffer += size;
        return A;
    }

    template <size_t N>
    inline Derived& operator>>(std::bitset<N>& C)
    {
        Impl::Bitset::deserialize(C, buffer);
        buffer += size_t(std::ceil(N / 8.0));
        return This();
    }
};

/// This class calculates size of any supported types
struct Calculator : CalculatorBase<Calculator>, SerializerOps<Calculator>
{
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};

/// This class serializes any of the supported types
struct Serializer : SerializerBase<Serializer>, SerializerOps<Serializer>
{
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};

/// This class deserializes any of the supported types
struct Deserializer : DeserializerBase<Deserializer>, DeserializerOps<Deserializer>
{
    using Base::Base;
    using Ops::operator>>;
    using Base::operator>>;
};

/// Array of 8 bit data type
using ByteArray = std::basic_string<char>;

/// Takes an unlimited number of arguements (serializable data types) and serializes them into a
/// byte array and returns the result
template <typename Head, typename... Tail>
ByteArray serialize(const Head& head, Tail&&... tail)
{
    Calculator calculator;
    calculator.process(head, std::forward<Tail>(tail)...);
    ByteArray data(calculator.size, 0);

    Serializer serializer{&data.front()};
    serializer.process(head, std::forward<Tail>(tail)...);
    return data;
}

/// Takes a byte array and an unlimited number of arguements and deserializes the data into the
/// arguements and returns the size of bytes consumed from byte array or char sequence
template <typename Head, typename... Tail>
size_t deserialize(const ByteArray& data, Head& head, Tail&&... tail)
{
    Deserializer deserializer{&data.front()};
    deserializer.process(head, std::forward<Tail>(tail)...);
    return size_t(deserializer.buffer - &data.front());
}

/// Takes a char sequence and an unlimited number of arguements and deserializes the data into the
/// arguements and returns the size of bytes consumed from byte array or char sequence
template <typename Head, typename... Tail>
size_t deserialize(const char* data, Head& head, Tail&&... tail)
{
    Deserializer deserializer{data};
    deserializer.process(head, std::forward<Tail>(tail)...);
    return size_t(deserializer.buffer - data);
}

/// Writes a byte array into a file and returns weather it succeeds or not
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

/// Reads binary data (the serialized data) from file into a byte array and returns weather it
/// succeeds or not
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

/// Does the job of serialize and write together. serializes arguements and writes them into a file
/// and returns weather it succeeds or not
template <typename Head, typename... Tail>
inline bool save(const std::string& path, const Head& head, Tail&&... tail)
{
    return write(path, serialize(head, std::forward<Tail>(tail)...));
}

/// Does the job of read and deserialize together. reads data from file and deserializes it into
/// arguements and returns weather it succeeds or not
template <typename Head, typename... Tail>
inline bool load(const std::string& path, Head& head, Tail&&... tail)
{
    ByteArray A;
    if (!read(path, A)) return false;
    return deserialize(A, head, std::forward<Tail>(tail)...) == A.size();
}
}  // namespace Serio

#undef SERIALIZER_CREATE_TYPE
#undef DESERIALIZER_CREATE_TYPE
#undef FUNC_FOR_TYPES

#endif  // SSERIALIZATION_H
