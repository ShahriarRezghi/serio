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

/// @brief Macro for registering custom classes and structures.
///
/// This macro simplifies the process of serializing and deserializing of custom classes. In order
/// to make nearly all of the custom classes usable This macro can be used (there are some rare
/// cases when you have to implement _serialize() and _deserialize() functions yourself).
///
/// Example:
/// @code
/// struct Point
/// {
///     int x, y;
///     SERIO_REGISTER(x, y)
/// };
/// @endcode
#define SERIO_REGISTER(...)              \
    template <typename Derived>          \
    friend class Serio::SerializerOps;   \
    template <typename Derived>          \
    friend class Serio::DeserializerOps; \
                                         \
    template <typename Serializer>       \
    void _serialize(Serializer& C) const \
    {                                    \
        C.process(__VA_ARGS__);          \
    }                                    \
    template <typename Deserializer>     \
    void _deserialize(Deserializer& C)   \
    {                                    \
        C.process(__VA_ARGS__);          \
    }

/// Main namespace of the library.
namespace Serio
{
#if SERIO_SIZE == 8
/// Yype of the size of containers and such that is serialized and deserialized.
using Size = uint64_t;
#elif SERIO_SIZE == 4
/// Type of the size of containers and such that is serialized and deserialized.
using Size = uint32_t;
#elif SERIO_SIZE == 2
/// Type of the size of containers and such that is serialized and deserialized.
using Size = uint16_t;
#else
static_assert(false, "SERIO_SIZE can only be 2, 4 or 8.");
using Size = uint64_t;
#endif

/// Container of single bytes.
using ByteArray = std::basic_string<char>;

///@brief Wrapper for raw array.
///
/// This class wraps the raw arrays so they can be serialized and deserialized too.
///
/// Example:
/// @code
/// int A[10];
/// Serio::ByteArray str = Serio::serialize(Serio::Array<int>(A, 10));
/// Serio::deserialize(str, Serio::Array<int>(A, 10));
/// @endcode
template <typename T, size_t Size>
struct Array
{
    T* data;
    Array(const T* data) : data(const_cast<T*>(data)) {}
};

bool _read(const std::string& path, std::basic_string<char>& data)
{
    std::basic_ifstream<char> stream(path, std::ios::binary | std::ios::in);
    if (!stream.is_open()) return false;

    stream.seekg(0, std::ios::end);
    auto size = stream.tellg();
    stream.seekg(0, std::ios::beg);
    data.assign(size_t(size), 0);

    if (stream.rdbuf()->sgetn(&data.front(), std::streamsize(size)) != size) return false;
    return true;
}

bool _write(const std::string& path, const std::basic_string<char>& data)
{
    std::basic_ofstream<char> stream(path, std::ios::binary | std::ios::out);
    if (!stream.is_open()) return false;

    auto size = stream.rdbuf()->sputn(data.data(), data.size());
    if (size != std::streamsize(data.size())) return false;
    return true;
}

template <typename T>
Size iteratableSize(const T& value)
{
    return Size(value.size());
}
template <typename... Ts>
Size iteratableSize(const std::forward_list<Ts...>& value)
{
    return Size(std::distance(value.begin(), value.end()));
}

namespace Number
{
bool little()
{
    short test = 1;
    auto ptr = reinterpret_cast<char*>(&test);
    return ptr[0];
}

template <size_t N, size_t I>
struct Number
{
    static void lserialize(const char* ptr, char* buffer)
    {
        Number<N, I - 1>::lserialize(ptr, buffer);
        buffer[I] = ptr[I];
    }
    static void bserialize(const char* ptr, char* buffer)
    {
        Number<N, I - 1>::bserialize(ptr, buffer);
        buffer[I] = ptr[N - I - 1];
    }
    static void deserialize(char* ptr, const char* buffer)
    {
        Number<N, I - 1>::deserialize(ptr, buffer);
        ptr[I] = buffer[I];
    }
};

template <size_t N>
struct Number<N, 0>
{
    static void lserialize(const char* ptr, char* buffer) { buffer[0] = ptr[0]; }
    static void bserialize(const char* ptr, char* buffer) { buffer[0] = ptr[N - 1]; }
    static void deserialize(char* ptr, const char* buffer) { ptr[0] = buffer[0]; }
};

template <typename T>
void serialize(const T& data, char* buffer)
{
    auto ptr = reinterpret_cast<const char*>(&data);
    if (little())
        Number<sizeof(T), sizeof(T) - 1>::lserialize(ptr, buffer);
    else
        Number<sizeof(T), sizeof(T) - 1>::bserialize(ptr, buffer);
}
template <typename T>
void deserialize(T& data, const char* buffer)
{
    auto ptr = reinterpret_cast<char*>(&data);
    Number<sizeof(T), sizeof(T) - 1>::deserialize(ptr, buffer);
}
}  // namespace Number

struct Bitset
{
    template <typename T>
    static void serialize(char* data, const T& value)
    {
        std::fill(data, data + size_t(std::ceil(value.size() / 8.0)), 0);
        for (size_t i = 0; i < value.size(); ++i) data[i / 8] |= value[i] << (i % 8);
    }
    template <typename T>
    static void deserialize(const char* data, T& value)
    {
        for (size_t i = 0; i < value.size(); ++i) value[i] = data[i / 8] & (1 << (i % 8));
    }
};

template <Size I>
struct Tuple
{
    template <typename Serializer, typename T>
    static void serialize(Serializer& item, const T& value)
    {
        Tuple<I - 1>::serialize(item, value);
        item << std::get<I>(value);
    }
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, T& value)
    {
        Tuple<I - 1>::deserialize(item, value);
        item >> std::get<I>(value);
    }
};
template <>
struct Tuple<0>
{
    template <typename Serializer, typename T>
    static void serialize(Serializer& item, const T& value)
    {
        item << std::get<0>(value);
    }
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, T& value)
    {
        item >> std::get<0>(value);
    }
};

#if __cplusplus >= 201703L
template <typename Tuple, Size I>
struct Variant
{
    template <typename Serializer, typename T>
    static void serialize(Serializer& item, const T& value)
    {
        Variant<Tuple, I - 1>::serialize(item, value);
        if (value.index() == I) item << std::get<I>(value);
    }
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, Size index, T& value)
    {
        Variant<Tuple, I - 1>::deserialize(item, index, value);
        if (index == I)
        {
            typename std::tuple_element<I, Tuple>::type elem;
            item >> elem;
            value = std::move(elem);
        }
    }
};
template <typename Tuple>
struct Variant<Tuple, 0>
{
    template <typename Serializer, typename T>
    static void serialize(Serializer& item, const T& value)
    {
        if (value.index() == 0) item << std::get<0>(value);
    }
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, Size index, T& value)
    {
        if (index == 0)
        {
            typename std::tuple_element<0, Tuple>::type elem;
            item >> elem;
            value = std::move(elem);
        }
    }
};
#endif

template <typename... Ts>
class Queue : public std::queue<Ts...>
{
public:
    using std::queue<Ts...>::c;
};
template <typename... Ts>
class Stack : public std::stack<Ts...>
{
public:
    using std::stack<Ts...>::c;
};
template <typename... Ts>
class PQueue : public std::priority_queue<Ts...>
{
public:
    using std::priority_queue<Ts...>::c;
};

/// @brief This class implements the size calculation of basic data types.
template <typename Derived>
class CalculatorBase
{
    Derived& This() { return *reinterpret_cast<Derived*>(this); }

public:
    size_t size;

    CalculatorBase(size_t size = 0) : size(size) {}

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&>::type operator<<(
        const T& C)
    {
        size += sizeof(C);
        return This();
    }

    template <typename Traits, typename Alloc>
    Derived& operator<<(const std::basic_string<char, Traits, Alloc>& C)
    {
        size += sizeof(Size) + C.size();
        return This();
    }

    template <size_t N>
    Derived& operator<<(const std::bitset<N>& C)
    {
        (void)C;
        size += Size(std::ceil(N / 8.0));
        return This();
    }

    template <typename Alloc>
    Derived& operator<<(const std::vector<bool, Alloc>& C)
    {
        size += sizeof(Size) + size_t(std::ceil(C.size() / 8.0));
        return This();
    }
};

/// @brief This class implements the serialization of basic data types to buffer.
template <typename Derived>
struct SerializerBase
{
    Derived& This() { return *reinterpret_cast<Derived*>(this); }

public:
    char* buffer;

    SerializerBase(char* buffer = nullptr) : buffer(buffer) {}

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&>::type operator<<(
        const T& C)
    {
        Number::serialize(C, buffer);
        buffer += sizeof(C);
        return This();
    }

    template <typename Traits, typename Alloc>
    Derived& operator<<(const std::basic_string<char, Traits, Alloc>& C)
    {
        This() << Size(C.size());
        if (C.size() == 0) return This();
        std::memcpy(buffer, C.data(), C.size());
        buffer += C.size();
        return This();
    }

    template <size_t N>
    Derived& operator<<(const std::bitset<N>& C)
    {
        Bitset::serialize(buffer, C);
        buffer += size_t(std::ceil(N / 8.0));
        return This();
    }

    template <typename Alloc>
    Derived& operator<<(const std::vector<bool, Alloc>& C)
    {
        This() << Size(C.size());
        Bitset::serialize(buffer, C);
        buffer += size_t(std::ceil(C.size() / 8.0));
        return This();
    }
};

/// @brief This class implements the deserialization of basic data types from buffer.
template <typename Derived>
struct DeserializerBase
{
    Derived& This() { return *reinterpret_cast<Derived*>(this); }

    template <typename T>
    T get()
    {
        T value;
        This() >> value;
        return value;
    }

public:
    const char* buffer;

    DeserializerBase(const char* buffer = nullptr) : buffer(buffer) {}

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&>::type operator>>(T& C)
    {
        Number::deserialize(C, buffer);
        buffer += sizeof(C);
        return This();
    }

    template <typename Traits, typename Alloc>
    Derived& operator>>(std::basic_string<char, Traits, Alloc>& C)
    {
        C.resize(this->get<Size>());
        if (C.size() == 0) return This();
        std::memcpy(&C.front(), buffer, C.size());
        buffer += C.size();
        return This();
    }

    template <size_t N>
    Derived& operator>>(std::bitset<N>& C)
    {
        Bitset::deserialize(buffer, C);
        buffer += size_t(std::ceil(N / 8.0));
        return This();
    }
    template <typename Alloc>
    Derived& operator>>(std::vector<bool, Alloc>& C)
    {
        C.resize(this->get<Size>());
        Bitset::deserialize(buffer, C);
        buffer += size_t(std::ceil(C.size() / 8.0));
        return This();
    }
};

/// @brief This class implements the serialization of basic data types to stream.
template <typename Derived>
struct StreamSerializerBase
{
    Derived& This() { return *reinterpret_cast<Derived*>(this); }

    char buffer[1024];

public:
    std::basic_ostream<char>* stream;

    StreamSerializerBase(std::basic_ostream<char>* stream) : stream(stream) {}

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&>::type operator<<(
        const T& C)
    {
        Number::serialize(C, buffer);
        stream->rdbuf()->sputn(buffer, sizeof(C));
        return This();
    }

    template <typename Traits, typename Alloc>
    Derived& operator<<(const std::basic_string<char, Traits, Alloc>& C)
    {
        This() << Size(C.size());
        if (C.size() == 0) return This();
        stream->rdbuf()->sputn(C.data(), C.size());
        return This();
    }

    template <size_t N>
    Derived& operator<<(const std::bitset<N>& C)
    {
        auto size = size_t(std::ceil(N / 8.0));
        if (size == 0) return This();
        ByteArray buffer(size, 0);
        Bitset::serialize(&buffer.front(), C);
        stream->rdbuf()->sputn(buffer.data(), buffer.size());
        return This();
    }

    template <typename Alloc>
    Derived& operator<<(const std::vector<bool, Alloc>& C)
    {
        This() << Size(C.size());
        if (C.size() == 0) return This();
        ByteArray buffer(size_t(std::ceil(C.size() / 8.0)), 0);
        Bitset::serialize(&buffer.front(), C);
        stream->rdbuf()->sputn(buffer.data(), buffer.size());
        return This();
    }
};

/// @brief This class implements the deserialization of basic data types to stream.
template <typename Derived>
struct StreamDeserializerBase
{
    Derived& This() { return *reinterpret_cast<Derived*>(this); }

    template <typename T>
    T get()
    {
        T value;
        This() >> value;
        return value;
    }

    char buffer[1024];

public:
    std::basic_istream<char>* stream;

    StreamDeserializerBase(std::basic_istream<char>* stream = nullptr) : stream(stream) {}

    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&>::type operator>>(T& C)
    {
        stream->rdbuf()->sgetn(buffer, sizeof(C));
        Number::deserialize(C, buffer);
        return This();
    }

    template <typename Traits, typename Alloc>
    Derived& operator>>(std::basic_string<char, Traits, Alloc>& C)
    {
        C.resize(this->get<Size>());
        if (C.size() == 0) return This();
        stream->rdbuf()->sgetn(&C.front(), C.size());
        return This();
    }

    template <size_t N>
    Derived& operator>>(std::bitset<N>& C)
    {
        auto size = size_t(std::ceil(N / 8.0));
        if (size == 0) return This();
        ByteArray buffer(size, 0);
        stream->rdbuf()->sgetn(&buffer.front(), buffer.size());
        Bitset::deserialize(buffer.data(), C);
        return This();
    }

    template <typename Alloc>
    Derived& operator>>(std::vector<bool, Alloc>& C)
    {
        C.resize(this->get<Size>());
        if (C.size() == 0) return This();
        ByteArray buffer(size_t(std::ceil(C.size() / 8.0)), 0);
        stream->rdbuf()->sgetn(&buffer.front(), buffer.size());
        Bitset::deserialize(buffer.data(), C);
        return This();
    }
};

/// @brief Processes more complex structures for serialization and size calculation.
///
/// This class breaks more complex structurs into simple elements and passes them on to the class that
/// handles them to calculate their size or serialize them.
template <typename Derived>
class SerializerOps
{
    Derived& This() { return *static_cast<Derived*>(this); }

    template <typename Iter>
    Derived& iteratable(const Iter& C)
    {
        This() << Size(iteratableSize(C));
        for (const auto& S : C) This() << S;
        return This();
    }

public:
    template <typename T>
    typename std::enable_if<std::is_class<T>::value, Derived&>::type operator<<(const T& C)
    {
        C._serialize(This());
        return This();
    }

    template <typename T, typename Alloc>
    Derived& operator<<(const std::vector<T, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T, typename Alloc>
    Derived& operator<<(const std::list<T, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T, typename Alloc>
    Derived& operator<<(const std::deque<T, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T, typename Traits, typename Alloc>
    Derived& operator<<(const std::basic_string<T, Traits, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T, typename Alloc>
    Derived& operator<<(const std::forward_list<T, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T>
    Derived& operator<<(const std::valarray<T>& C)
    {
        return iteratable(C);
    }

    template <typename T, size_t N>
    Derived& operator<<(const std::array<T, N>& C)
    {
        for (const auto& S : C) This() << S;
        return This();
    }
    template <typename T, size_t S>
    Derived& operator<<(const Array<T, S>& C)
    {
        for (size_t i = 0; i < S; ++i) This() << C.data[i];
        return This();
    }

    template <typename T, typename Sequence>
    Derived& operator<<(const std::queue<T, Sequence>& C)
    {
        return This() << reinterpret_cast<const Queue<T, Sequence>&>(C).c;
    }
    template <typename T, typename Sequence>
    Derived& operator<<(const std::priority_queue<T, Sequence>& C)
    {
        return This() << reinterpret_cast<const PQueue<T, Sequence>&>(C).c;
    }
    template <typename T, typename Sequence>
    Derived& operator<<(const std::stack<T, Sequence>& C)
    {
        return This() << reinterpret_cast<const Stack<T, Sequence>&>(C).c;
    }

    template <typename T, typename Comp, typename Alloc>
    Derived& operator<<(const std::set<T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator<<(const std::unordered_set<T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T, typename Comp, typename Alloc>
    Derived& operator<<(const std::multiset<T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator<<(const std::unordered_multiset<T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename K, typename T, typename Comp, typename Alloc>
    Derived& operator<<(const std::map<K, T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator<<(const std::unordered_map<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename K, typename T, typename Comp, typename Alloc>
    Derived& operator<<(const std::multimap<K, T, Comp, Alloc>& C)
    {
        return iteratable(C);
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator<<(const std::unordered_multimap<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable(C);
    }

    template <typename T1, typename T2>
    Derived& operator<<(const std::pair<T1, T2>& C)
    {
        return This() << C.first << C.second;
    }
    template <typename... Ts>
    Derived& operator<<(const std::tuple<Ts...>& C)
    {
        Tuple<sizeof...(Ts) - 1>::serialize(This(), C);
        return This();
    }

    template <typename T>
    Derived& operator<<(const std::shared_ptr<T>& C)
    {
        This() << bool(C);
        if (C) This() << *C.get();
        return This();
    }
    template <typename T, typename Deleter>
    Derived& operator<<(const std::unique_ptr<T, Deleter>& C)
    {
        This() << bool(C);
        if (C) This() << *C.get();
        return This();
    }

    template <typename T>
    Derived& operator<<(const std::complex<T>& C)
    {
        return This() << C.real() << C.imag();
    }
    template <typename Clock, typename Dur>
    Derived& operator<<(const std::chrono::time_point<Clock, Dur>& C)
    {
        using rep = typename std::chrono::time_point<Clock, Dur>::rep;
        return This() << reinterpret_cast<const rep&>(C);
    }

#if __cplusplus >= 201703L
    template <typename T>
    Derived& operator<<(const std::optional<T>& C)
    {
        This() << C.has_value();
        if (C.has_value()) This() << C.value();
        return This();
    }
    template <typename... Ts>
    Derived& operator<<(const std::variant<Ts...>& C)
    {
        This() << Size(C.index());
        Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::serialize(This(), C);
        return This();
    }
#endif

    template <typename Head, typename... Tail>
    Derived& process(const Head& head, Tail&&... tail)
    {
        This() << head;
        return process(std::forward<Tail>(tail)...);
    }
    Derived& process() { return This(); }
};

/// @brief Processes more complex structures for deserialization.
///
/// This class more complex structurs into simple elements and passes them on to the class that
/// handles them to deserialize.
template <typename Derived>
class DeserializerOps
{
    Derived& This() { return *static_cast<Derived*>(this); }

    template <typename T>
    T get()
    {
        T value;
        This() >> value;
        return value;
    }

    template <typename Iter, typename T>
    Derived& assignable(Iter& C)
    {
        C.resize(this->get<Size>());
        for (auto& value : C) This() >> value;
        return This();
    }

    template <typename Iter, typename T>
    Derived& iteratable(Iter& C)
    {
        C.clear();
        auto it = C.begin();
        auto size = this->get<Size>();
        for (Size i = 0; i < size; ++i) it = C.emplace_hint(it, this->get<T>());
        return This();
    }

    template <typename Ptr, typename T>
    Derived& pointer(Ptr& C)
    {
        if (this->get<bool>())
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
    typename std::enable_if<std::is_class<T>::value, Derived&>::type operator>>(T& C)
    {
        C._deserialize(This());
        return This();
    }

    template <typename T, typename Alloc>
    Derived& operator>>(std::vector<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }
    template <typename T, typename Alloc>
    Derived& operator>>(std::list<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }
    template <typename T, typename Alloc>
    Derived& operator>>(std::deque<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }
    template <typename T, typename Traits, typename Alloc>
    Derived& operator>>(std::basic_string<T, Traits, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }
    template <typename T, typename Alloc>
    Derived& operator>>(std::forward_list<T, Alloc>& C)
    {
        return assignable<decltype(C), T>(C);
    }
    template <typename T>
    Derived& operator>>(std::valarray<T>& C)
    {
        return assignable<decltype(C), T>(C);
    }

    template <typename T, size_t N>
    Derived& operator>>(std::array<T, N>& C)
    {
        for (auto& S : C) This() >> S;
        return This();
    }
    template <typename T, size_t S>
    Derived& operator>>(Array<T, S> C)
    {
        for (size_t i = 0; i < S; ++i) This() >> C.data[i];
        return This();
    }

    template <typename T, typename Sequence>
    Derived& operator>>(std::queue<T, Sequence>& C)
    {
        return This() >> reinterpret_cast<Queue<T, Sequence>&>(C).c;
    }
    template <typename T, typename Sequence>
    Derived& operator>>(std::priority_queue<T, Sequence>& C)
    {
        return This() >> reinterpret_cast<PQueue<T, Sequence>&>(C).c;
    }
    template <typename T, typename Sequence>
    Derived& operator>>(std::stack<T, Sequence>& C)
    {
        return This() >> reinterpret_cast<Stack<T, Sequence>&>(C).c;
    }

    template <typename T, typename Comp, typename Alloc>
    Derived& operator>>(std::set<T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }
    template <typename T, typename Comp, typename Alloc>
    Derived& operator>>(std::multiset<T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }
    template <typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator>>(std::unordered_set<T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }
    template <typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator>>(std::unordered_multiset<T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), T>(C);
    }

    template <typename K, typename T, typename Comp, typename Alloc>
    Derived& operator>>(std::map<K, T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator>>(std::unordered_map<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }
    template <typename K, typename T, typename Comp, typename Alloc>
    Derived& operator>>(std::multimap<K, T, Comp, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }
    template <typename K, typename T, typename Hash, typename Pred, typename Alloc>
    Derived& operator>>(std::unordered_multimap<K, T, Hash, Pred, Alloc>& C)
    {
        return iteratable<decltype(C), std::pair<K, T>>(C);
    }

    template <typename T1, typename T2>
    Derived& operator>>(std::pair<T1, T2>& C)
    {
        return This() >> C.first >> C.second;
    }
    template <typename... Ts>
    Derived& operator>>(std::tuple<Ts...>& C)
    {
        Tuple<sizeof...(Ts) - 1>::deserialize(This(), C);
        return This();
    }

    template <typename T>
    Derived& operator>>(std::shared_ptr<T>& C)
    {
        return pointer<decltype(C), T>(C);
    }
    template <typename T, typename Deleter>
    Derived& operator>>(std::unique_ptr<T, Deleter>& C)
    {
        return pointer<decltype(C), T>(C);
    }

    template <typename T>
    Derived& operator>>(std::complex<T>& C)
    {
        auto R = this->get<T>(), I = this->get<T>();
        C = std::complex<T>(R, I);
        return This();
    }
    template <typename Clock, typename Dur>
    Derived& operator>>(std::chrono::time_point<Clock, Dur>& C)
    {
        using rep = typename std::chrono::time_point<Clock, Dur>::rep;
        return This() >> reinterpret_cast<rep&>(C);
    }

#if __cplusplus >= 201703L
    template <typename T>
    Derived& operator>>(std::optional<T>& C)
    {
        if (this->get<bool>())
            C.emplace(this->get<T>());
        else
            C.reset();

        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::variant<Ts...>& C)
    {
        auto index = this->get<Size>();
        Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(This(), index, C);
        return This();
    }
#endif

    template <typename Head, typename... Tail>
    Derived& process(Head& head, Tail&&... tail)
    {
        This() >> head;
        return process(std::forward<Tail>(tail)...);
    }
    template <typename T, size_t S, typename... Tail>
    Derived& process(Array<T, S> head, Tail&&... tail)
    {
        This() >> std::move(head);
        return process(std::forward<Tail>(tail)...);
    }
    Derived& process() { return This(); }
};

/// @brief This class calculates size of any of the supported types.
struct Calculator : CalculatorBase<Calculator>, SerializerOps<Calculator>
{
    using Base = CalculatorBase<Calculator>;
    using Ops = SerializerOps<Calculator>;
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};

/// @brief This class serializes any of the supported types to buffer.
struct Serializer : SerializerBase<Serializer>, SerializerOps<Serializer>
{
    using Base = SerializerBase<Serializer>;
    using Ops = SerializerOps<Serializer>;
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};

/// @brief This class deserializes any of the supported types from buffer.
struct Deserializer : DeserializerBase<Deserializer>, DeserializerOps<Deserializer>
{
    using Base = DeserializerBase<Deserializer>;
    using Ops = DeserializerOps<Deserializer>;
    using Base::Base;
    using Ops::operator>>;
    using Base::operator>>;
};

/// @brief This class serializes any of the supported types to stream.
struct StreamSerializer : StreamSerializerBase<StreamSerializer>, SerializerOps<StreamSerializer>
{
    using Base = StreamSerializerBase<StreamSerializer>;
    using Ops = SerializerOps<StreamSerializer>;
    using Base::Base;
    using Ops::operator<<;
    using Base::operator<<;
};

/// @brief This class deserializes any of the supported types from stream.
struct StreamDeserializer : StreamDeserializerBase<StreamDeserializer>, DeserializerOps<StreamDeserializer>
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
/// @param ts Parameters to be serialized
/// @returns number of bytes that the input arguments will consume
/// @see deserialize()
template <typename... Ts>
size_t size(Ts&&... ts)
{
    return Calculator().process(std::forward<Ts>(ts)...).size;
}

/// Serializes and unlimited number of input arguments (serializable data types).
///
/// Example:
/// @code
/// int A = 0, B = 0;
/// auto data = new char[Serio::size(A, B)];
/// Serio::fill(data, A, B);
/// @endcode
///
/// @param ts Parameters to be serialized
/// @param tail Rest of the parameters to be serialized
/// @returns The number of bytes written to sequence
/// @see serialize()
template <typename... Ts>
size_t fill(char* data, Ts&&... ts)
{
    return size_t(Serializer(data).process(std::forward<Ts>(ts)...).buffer - data);
}

/// Deserializes and unlimited number of input arguments (deserializable data types).
///
/// Example:
/// @code
/// int A, B;
/// char* data = "<serialized-data>";
/// size_t consumed = Serio::deserialize(data, A, B);
/// @endcode
///
/// @param data Sequence of bytes containing serialized data
/// @param ts Parameters to be deserialized
/// @returns The number of bytes consumed from char sequence
/// @see serialize(), deserialize()
template <typename... Ts>
size_t deserialize(const char* data, Ts&&... ts)
{
    return size_t(Deserializer(data).process(std::forward<Ts>(ts)...).buffer - data);
}

/// Serializes and unlimited number of input arguments (serializable data types).
///
/// Example:
/// @code
/// int A = 0, B = 0;
/// Serio::ByteArray str = Serio::serialize(A, B);
/// @endcode
///
/// @param ts Parameters to be serialized
/// @returns Array of bytes containing the serialized data
/// @see deserialize()
template <typename... Ts>
ByteArray serialize(Ts&&... ts)
{
    ByteArray data(Serio::size(std::forward<Ts>(ts)...), 0);
    Serio::fill(&data.front(), std::forward<Ts>(ts)...);
    return data;
}

/// Deserializes and unlimited number of input arguments (deserializable data types).
///
/// Example:
/// @code
/// int A, B;
/// Serio::ByteArray str = "<serialized-data>";
/// size_t consumed = Serio::deserialize(str, A, B);
/// @endcode
///
/// @param data Array of bytes containing serialized data
/// @param ts Parameters to be deserialized
/// @returns The number of bytes consumed from byte array
/// @see serialize(), deserialize()
template <typename... Ts>
size_t deserialize(const ByteArray& data, Ts&&... ts)
{
    return deserialize(&data.front(), std::forward<Ts>(ts)...);
}

/// Serializes and unlimited number of input arguments (serializable data types) and writes it to a
/// file.
///
/// Example:
/// @code
/// int A = 0, B = 0;
/// bool success = Serio::save("<file-path>", A, B);
/// @endcode
///
/// @param path Path of the file to be writen.
/// @param ts Parameters to be serialized
/// @returns True if succeeds otherwise false
/// @see serialize()
template <typename... Ts>
bool save(const std::string& path, Ts&&... ts)
{
    return _write(path, serialize(std::forward<Ts>(ts)...));
}

/// Reads data from a file and deserializes and unlimited number of input arguments (deserializable
/// data types).
///
/// Example:
/// @code
/// int A, B;
/// bool success = Serio::load("<file-path>", A, B);
/// @endcode
///
/// @param path Path of the file to be read
/// @param ts Parameters to be deserialized
/// @returns True if succeeds otherwise false
/// @see deserialize()
template <typename... Ts>
bool load(const std::string& path, Ts&&... ts)
{
    ByteArray A;
    if (!_read(path, A)) return false;
    return deserialize(A, std::forward<Ts>(ts)...) == A.size();
}

/// Serializes and unlimited number of input arguments (serializable data types).
///
/// Example:
/// @code
/// int A = 0, B = 0;
/// std::ostringstream stream;
/// Serio::write(&stream, A, B);
/// @endcode
///
/// @param stream Pointer to the output stream of chars
/// @param ts Parameters to be serialized
/// @see streamDeserialize()
template <typename... Ts>
void write(std::basic_ostream<char>* stream, Ts&&... ts)
{
    StreamSerializer serializer(stream);
    serializer.process(std::forward<Ts>(ts)...);
}

/// Deserializes and unlimited number of input arguments (deserializable data types).
///
/// Example:
/// @code
/// int A, B;
/// std::istringstream stream;  // contains serialized data
/// Serio::read(&stream, A, B);
/// @endcode
///
/// @param stream Pointer to the input stream of chars
/// @param data Array of bytes containing serialized data
/// @param ts Parameters to be deserialized
/// @see serialize(), deserialize()
template <typename... Ts>
void read(std::basic_istream<char>* stream, Ts&&... ts)
{
    StreamDeserializer deserializer(stream);
    deserializer.process(std::forward<Ts>(ts)...);
}
}  // namespace Serio

#endif  // SSERIALIZATION_H
