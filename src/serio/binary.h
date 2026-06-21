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

#pragma once

#include <serio/tools.h>

#include <algorithm>
#include <atomic>
#include <bitset>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstring>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
//
#if SERIO_CPP_VERSION >= 201703L
#include <string_view>
#include <variant>
#endif
//
#if SERIO_CPP_VERSION >= 202002L
#include <span>
#endif

#if SERIO_CPP_VERSION >= 202302L
#include <expected>
#endif

namespace Serio
{
namespace Impl
{
template <typename Derived>
class SerializerBase
{
    Derived& This() { return (Derived&)*this; }

    char* take(Size size)
    {
        Size len = buffer.size();
        buffer.append(size, 0);
        return (char*)buffer.data() + len;
    }

    std::string& buffer;

public:
    SerializerBase(std::string& buffer) : buffer(buffer) { buffer.reserve(1024); }

    template <typename T>
    EnableIfT<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&> operator<<(const T& value)
    {
        BasicType::serialize(take(sizeof(value)), value);
        return This();
    }
    template <typename... Ts>
    Derived& operator<<(const std::basic_string<char, Ts...>& value)
    {
        This() << containerSize(value);
        this->write(value.data(), value.size());
        return This();
    }
    template <size_t N>
    Derived& operator<<(const std::bitset<N>& value)
    {
        Bitset::serialize(take((N + 7) / 8), value);
        return This();
    }
    template <typename... Ts>
    Derived& operator<<(const std::vector<bool, Ts...>& value)
    {
        This() << Size(value.size());
        Bitset::serialize(take((value.size() + 7) / 8), value);
        return This();
    }
    void write(const void* data, Size len) { buffer.append((const char*)data, len); }

#if SERIO_CPP_VERSION >= 201703L
    template <typename... Ts>
    Derived& operator<<(const std::basic_string_view<char, Ts...>& value)
    {
        This() << containerSize(value);
        this->write(value.data(), value.size());
        return This();
    }
#endif
};

template <typename Derived>
struct StreamSerializerBase
{
    std::vector<char> _buffer;
    std::ostream& _stream;
    Derived& This() { return (Derived&)*this; }

public:
    StreamSerializerBase(std::ostream& stream) : _stream(stream) { _buffer.resize(16); }

    template <typename T>
    EnableIfT<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&> operator<<(const T& value)
    {
        BasicType::serialize(_buffer.data(), value);
        write(_buffer.data(), sizeof(value));
        return This();
    }
    template <typename... Ts>
    Derived& operator<<(const std::basic_string<char, Ts...>& value)
    {
        auto size = value.size();
        This() << Size(size);
        write(value.data(), value.size());
        return This();
    }
    template <size_t N>
    Derived& operator<<(const std::bitset<N>& value)
    {
        auto size = size_t((N + 7) / 8);
        if (size == 0) return This();
        if (size > _buffer.size()) _buffer.resize(size);
        Bitset::serialize(&_buffer.front(), value);
        write(_buffer.data(), size);
        return This();
    }
    template <typename... Ts>
    Derived& operator<<(const std::vector<bool, Ts...>& value)
    {
        This() << Size(value.size());
        if (value.size() == 0) return This();
        auto size = size_t((value.size() + 7) / 8);
        if (size > _buffer.size()) _buffer.resize(size);
        Bitset::serialize(&_buffer.front(), value);
        write(_buffer.data(), size);
        return This();
    }
    void write(const void* data, Size len)  //
    {
        if (len > 0) _stream.rdbuf()->sputn((const char*)data, len);
    }

#if SERIO_CPP_VERSION >= 201703L
    template <typename... Ts>
    Derived& operator<<(const std::basic_string_view<char, Ts...>& value)
    {
        This() << containerSize(value);
        this->write(value.data(), value.size());
        return This();
    }
#endif
};

template <typename Derived>
class DeserializerBase
{
    const char* start{nullptr};
    const char* buffer{nullptr};
    Size length{0};
    Size maxLength{0};

    template <typename T>
    T get()
    {
        T value;
        This() >> value;
        return value;
    }

    void advance(Size size)
    {
        this->buffer += size;
        this->length -= size;
    }

    Derived& This() { return (Derived&)*this; }

public:
    DeserializerBase(const char* ptr, Size length, Size maxLength)
        : start(ptr), buffer(ptr), length(length), maxLength(maxLength)
    {
    }

    Size progress() { return buffer - start; }

    Size getLength()
    {
        auto size = this->get<Size>();
        if (maxLength > 0) SERIO_ASSERT(size <= maxLength, "Requested container size above maximum limit");
        return size;
    }

    template <typename T>
    EnableIfT<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&> operator>>(T& value)
    {
        SERIO_ASSERT(length >= sizeof(value), "Requested structure doesn't match the input buffer");
        BasicType::deserialize(this->buffer, value);
        advance(sizeof(value));
        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::basic_string<char, Ts...>& value)
    {
        value.resize(getLength());
        SERIO_ASSERT(length >= value.size(), "Requested structure doesn't match the input buffer");
        std::copy(this->buffer, this->buffer + value.size(), &value[0]);
        advance(value.size());
        return This();
    }
    template <size_t N>
    Derived& operator>>(std::bitset<N>& value)
    {
        size_t size = (N + 7) / 8;
        SERIO_ASSERT(length >= size, "Requested structure doesn't match the input buffer");
        Bitset::deserialize(this->buffer, value);
        advance(size);
        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::vector<bool, Ts...>& value)
    {
        value.resize(getLength());
        size_t size = (value.size() + 7) / 8;
        SERIO_ASSERT(length >= size, "Requested structure doesn't match the input buffer");
        Bitset::deserialize(this->buffer, value);
        advance(size);
        return This();
    }
    template <typename T>
    void read(T* data, Size len)
    {
        len *= sizeof(T);
        SERIO_ASSERT(length >= len, "Requested structure doesn't match the input buffer");
        std::memcpy((void*)data, this->buffer, len);
        advance(len);
    }
};

template <typename Derived>
struct StreamDeserializerBase
{
    std::vector<char> _buffer;
    std::istream& _stream;
    Size _maxLength{0};

    template <typename T>
    T get()
    {
        T value;
        This() >> value;
        return value;
    }

    Derived& This() { return (Derived&)*this; }

public:
    StreamDeserializerBase(std::istream& stream, Size maxLength) : _stream(stream), _maxLength(maxLength)
    {
        _buffer.resize(16);
    }

    Size getLength()
    {
        auto size = this->get<Size>();
        if (_maxLength > 0) SERIO_ASSERT(size <= _maxLength, "Requested container size above maximum limit");
        return size;
    }

    template <typename T>
    EnableIfT<std::is_arithmetic<T>::value || std::is_enum<T>::value, Derived&> operator>>(T& value)
    {
        read(_buffer.data(), sizeof(value));
        BasicType::deserialize(_buffer.data(), value);
        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::basic_string<char, Ts...>& value)
    {
        auto size = getLength();
        value.resize(size);
        if (size > 0) read(&value.front(), value.size());
        return This();
    }
    template <size_t N>
    Derived& operator>>(std::bitset<N>& value)
    {
        auto size = size_t((N + 7) / 8);
        if (size == 0) return This();
        if (size > _buffer.size()) _buffer.resize(size);
        read(&_buffer.front(), size);
        Bitset::deserialize(_buffer.data(), value);
        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::vector<bool, Ts...>& value)
    {
        value.resize(getLength());
        if (value.size() == 0) return This();
        auto size = size_t((value.size() + 7) / 8);
        if (size > _buffer.size()) _buffer.resize(size);
        read(&_buffer.front(), size);
        Bitset::deserialize(_buffer.data(), value);
        return This();
    }
    template <typename T>
    void read(T* data, Size len)
    {
        len *= sizeof(T);
        auto size = _stream.rdbuf()->sgetn((char*)data, len);
        SERIO_ASSERT(size == len, "Requested structure doesn't match the input stream");
    }
};

template <typename Derived>
class SerializerOps : public Base::Serializer, public Base::Binary
{
    Derived& This() { return (Derived&)*this; }

public:
    template <typename T>
    EnableIfT<std::is_class<T>::value && !IsFixed<T>::value && !IsResizable<T>::value && !IsAppendable<T>::value &&
                  !IsPointer<T>::value && !IsOptional<T>::value,
              Derived&>
    operator<<(const T& value)
    {
        CustomClass<T>().serialize(value, This());
        return This();
    }
    template <typename T>
    EnableIfT<IsContinuous<T>::value, Derived&> operator<<(const T& value)
    {
        if (!IsFixed<T>::value) This() << containerSize(value);
        if (littleEndian())
            This().write(value.data(), value.size() * sizeof(typename T::value_type));
        else
            for (const auto& item : value) This() << item;
        return This();
    }
    template <typename T>
    EnableIfT<(IsFixed<T>::value || IsResizable<T>::value || IsAppendable<T>::value) && !IsContinuous<T>::value,
              Derived&>
    operator<<(const T& value)
    {
        if (!IsFixed<T>::value) This() << containerSize(value);
        for (const auto& item : value) This() << item;
        return This();
    }
    template <typename T>
    EnableIfT<IsPointer<T>::value, Derived&> operator<<(const T& value)
    {
        This() << bool(value);
        if (value) This() << *value.get();
        return This();
    }
    template <typename... Ts>
    Derived& operator<<(const std::chrono::duration<Ts...>& value)
    {
        return This() << value.count();
    }
    template <typename... Ts>
    Derived& operator<<(const std::chrono::time_point<Ts...>& value)
    {
        return This() << value.time_since_epoch();
    }
    template <typename T>
    Derived& operator<<(const std::weak_ptr<T>& value)
    {
        return This() << value.lock();
    }
    template <typename... Ts>
    Derived& operator<<(const std::queue<Ts...>& value)
    {
        return This() << static_cast<const Queue<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Derived& operator<<(const std::stack<Ts...>& value)
    {
        return This() << static_cast<const Stack<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Derived& operator<<(const std::priority_queue<Ts...>& value)
    {
        return This() << static_cast<const PQueue<Ts...>&>(value).c;
    }
    template <typename T1, typename T2>
    Derived& operator<<(const std::pair<T1, T2>& value)
    {
        return This() << value.first << value.second;
    }
    template <typename... Ts>
    Derived& operator<<(const std::tuple<Ts...>& value)
    {
        Tuple<sizeof...(Ts)>::serialize(This(), value);
        return This();
    }
    template <typename T>
    Derived& operator<<(const std::complex<T>& value)
    {
        return This() << value.real() << value.imag();
    }
    template <typename... Ts>
    Derived& operator<<(const std::atomic<Ts...>& value)
    {
        return This() << value.load();
    }
    template <typename T>
    Derived& operator<<(const BinaryString<T>& value)
    {
        return This() << value.value;
    }
    template <typename T>
    Derived& operator<<(const NVP<T>& value)
    {
        return This() << value.value;
    }

#if SERIO_CPP_VERSION >= 201703L
    template <typename T>
    EnableIfT<IsOptional<T>::value, Derived&> operator<<(const T& value)
    {
        This() << value.has_value();
        if (value.has_value()) This() << value.value();
        return This();
    }
    template <typename... Ts>
    Derived& operator<<(const std::variant<Ts...>& value)
    {
        This() << Size(value.index());
        std::visit([&](auto&& arg) { This() << arg; }, value);
        return This();
    }
    Derived& operator<<(const std::monostate&)  //
    {
        return This();
    }
#endif

#if SERIO_CPP_VERSION >= 202302L
    template <typename... Ts>
    Derived& operator<<(const std::expected<Ts...>& value)  //
    {
        This() << value.has_value();

        if (value.has_value())
            return This() << value.value();
        else
            return This() << value.error();
    }
#endif

#if SERIO_ENABLE_FILESYSTEM
    Derived& operator<<(const std::filesystem::path& value)  //
    {
        return This() << value.string();
    }
#endif

    template <typename Head, typename... Tail>
    Derived& process(Head&& head, Tail&&... tail)
    {
        This() << std::forward<Head>(head);
        return process(std::forward<Tail>(tail)...);
    }
    Derived& process() { return This(); }
};

template <typename Derived>
class DeserializerOps : public Base::Deserializer, public Base::Binary
{
    Derived& This() { return (Derived&)*this; }

public:
    template <typename T>
    T get()
    {
        T value;
        This() >> value;
        return value;
    }

    template <typename T>
    EnableIfT<std::is_class<T>::value && !IsFixed<T>::value && !IsResizable<T>::value && !IsAppendable<T>::value &&
                  !IsPointer<T>::value && !IsOptional<T>::value,
              Derived&>
    operator>>(T& value)
    {
        CustomClass<T>().deserialize(value, This());
        return This();
    }
    template <typename T>
    EnableIfT<IsFixedContinuous<T>::value, Derived&> operator>>(T& value)
    {
        if (littleEndian())
            This().read(value.data(), value.size());
        else
            for (auto& item : value) This() >> item;
        return This();
    }
    template <typename T>
    EnableIfT<IsContinuous<T>::value && !IsFixedContinuous<T>::value, Derived&> operator>>(T& value)
    {
        value.resize(This().getLength());
        if (littleEndian())
            This().read(value.data(), value.size());
        else
            for (auto& item : value) This() >> item;
        return This();
    }
    template <typename T>
    EnableIfT<IsFixed<T>::value && !IsContinuous<T>::value, Derived&> operator>>(T& value)
    {
        for (auto& item : value) This() >> item;
        return This();
    }
    template <typename T>
    EnableIfT<IsResizable<T>::value && !IsContinuous<T>::value, Derived&> operator>>(T& value)
    {
        value.resize(This().getLength());
        for (auto& item : value) This() >> item;
        return This();
    }
    template <typename T>
    EnableIfT<IsAppendable<T>::value, Derived&> operator>>(T& value)
    {
        using Type = typename ValueType<T>::Type;
        value.clear();
        auto size = This().getLength();
        for (Size i = 0; i < size; ++i) value.emplace_hint(value.end(), this->get<Type>());
        return This();
    }
    template <typename T, Size N>
    Derived& operator>>(StaticArrayView<T, N> value)
    {
        if (std::is_arithmetic<T>::value && littleEndian())
            This().read(value.data(), value.size());
        else
            for (auto& item : value) This() >> item;
        return This();
    }
    template <typename T>
    Derived& operator>>(PointerView<T>& value)
    {
        static_assert(std::is_same<decltype(value), PointerView<T>>::value, "Pointer view is not deserializable.");
        return This();
    }
    template <typename T>
    EnableIfT<IsPointer<T>::value, Derived&> operator>>(T& value)
    {
        if (this->get<bool>())
        {
            auto* item = new typename std::remove_const<typename T::element_type>::type;
            This() >> *item;
            value.reset(item);
        }
        else
            value.reset();

        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::chrono::duration<Ts...>& value)
    {
        using Type = std::chrono::duration<Ts...>;
        auto item = this->get<typename Type::rep>();
        value = decltype(value)(item);
        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::chrono::time_point<Ts...>& value)
    {
        using Type = std::chrono::time_point<Ts...>;
        auto item = this->get<typename Type::duration>();
        value = decltype(value)(item);
        return This();
    }
    template <typename T>
    Derived& operator>>(std::weak_ptr<T>& value)
    {
        static_assert(std::is_same<decltype(value), std::weak_ptr<T>>::value, "Weak pointer is not deserializable.");
        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::queue<Ts...>& value)
    {
        return This() >> static_cast<Queue<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Derived& operator>>(std::stack<Ts...>& value)
    {
        return This() >> static_cast<Stack<Ts...>&>(value).c;
    }
    template <typename... Ts>
    Derived& operator>>(std::priority_queue<Ts...>& value)
    {
        return This() >> static_cast<PQueue<Ts...>&>(value).c;
    }
    template <typename T1, typename T2>
    Derived& operator>>(std::pair<T1, T2>& value)
    {
        return This() >> value.first >> value.second;
    }
    template <typename... Ts>
    Derived& operator>>(std::tuple<Ts...>& value)
    {
        Tuple<sizeof...(Ts)>::deserialize(This(), value);
        return This();
    }
    template <typename T>
    Derived& operator>>(std::complex<T>& value)
    {
        value.real(get<T>());
        value.imag(get<T>());
        return This();
    }
    template <typename T>
    Derived& operator>>(std::atomic<T>& value)
    {
        value.store(this->get<T>());
        return This();
    }
    template <typename T>
    Derived& operator>>(BinaryString<T> value)
    {
        return This() >> value.value;
    }
    template <typename T>
    Derived& operator>>(NVP<T> value)
    {
        return This() >> value.value;
    }

#if SERIO_CPP_VERSION >= 201703L
    template <typename... Ts>
    Derived& operator>>(std::basic_string_view<Ts...>& value)
    {
        static_assert(std::is_same<decltype(value), std::basic_string_view<Ts...>>::value,
                      "String view is not deserializable.");
        return This();
    }
    template <typename T>
    EnableIfT<IsOptional<T>::value, Derived&> operator>>(T& value)
    {
        if (this->get<bool>())
            value.emplace(this->get<typename T::value_type>());
        else
            value.reset();

        return This();
    }
    template <typename... Ts>
    Derived& operator>>(std::variant<Ts...>& value)
    {
        auto index = this->get<Size>();
        SERIO_ASSERT(index < sizeof...(Ts), "Invalid variant index");
        Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(This(), index, value);
        return This();
    }
    Derived& operator>>(std::monostate& value)
    {
        (void)value;
        return This();
    }
#endif

#if SERIO_CPP_VERSION >= 202002L
    template <typename T, size_t S>
    Derived& operator>>(std::span<T, S>& value)
    {
        static_assert(std::is_same<decltype(value), std::span<T, S>>::value, "Span is not deserializable.");
        return This();
    }
#endif

#if SERIO_CPP_VERSION >= 202302L
    template <typename T, typename E>
    Derived& operator>>(std::expected<T, E>& value)
    {
        if (this->get<bool>())
            value.emplace(this->get<T>());
        else
            value = std::unexpected(this->get<E>());

        return This();
    }
#endif

#if SERIO_ENABLE_FILESYSTEM
    Derived& operator>>(std::filesystem::path& value)  //
    {
        value = std::filesystem::path{this->get<std::string>()};
        return This();
    }
#endif

    template <typename Head, typename... Tail>
    Derived& process(Head&& head, Tail&&... tail)
    {
        This() >> std::forward<Head>(head);
        return process(std::forward<Tail>(tail)...);
    }
    Derived& process() { return This(); }
};

/// @brief This class serializes any of the supported types to buffer.
struct Serializer : SerializerBase<Serializer>, SerializerOps<Serializer>
{
    using Base = SerializerBase<Serializer>;
    using Ops = SerializerOps<Serializer>;
    using Base::SerializerBase;
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
    using Ops::get;
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
    using Ops::get;
};
}  // namespace Impl
}  // namespace Serio
