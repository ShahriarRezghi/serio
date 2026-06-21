/*
  Copyright (c) 2014, Randolph Voorhies, Shane Grant
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of the copyright holder nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <doctest/doctest.h>
#include <serio/serio.h>

#include <limits>
#include <random>

namespace Serio
{
template <Type type>
struct Serializer
{
};

template <>
struct Serializer<Type::Binary>
{
    Serializer(const SerializeOptions& options = {}) : options(options)
    {
        bool secondary = (options.compressLevel > -1) ^ (!options.encryptPassword.empty());
        header = 4 + (options.enableChecksum ? 4 : 0);
        array = secondary ? &_buffer : &buffer;
        buffer.resize(header);
    }

    template <typename T>
    Serializer& operator<<(T&& value)
    {
        Impl::Serializer(*array).process(std::forward<T>(value));
        return *this;
    }

    std::string& finalize()
    {
        bool checksum = options.enableChecksum;
        bool compress = options.compressLevel > -1;
        bool encrypt = !options.encryptPassword.empty();

        if (compress && !encrypt)
            Impl::compress(_buffer, buffer, options.compressLevel, header);
        else if (!compress && encrypt)
            Impl::encrypt(_buffer, buffer, options.encryptPassword, header);
        else if (compress && encrypt)
        {
            Impl::compress(buffer, _buffer, options.compressLevel, 0);
            Impl::encrypt(_buffer, buffer, options.encryptPassword, header);
        }

        uint32_t crc = 0;
        if (checksum) crc = Impl::crcCreate(StringView(buffer).view(header));
        Impl::writeHeader(&buffer.front(), checksum, compress, encrypt, crc);
        return buffer;
    }

private:
    std::string buffer;
    std::string _buffer;
    std::string* array;
    Size header = 0;
    SerializeOptions options;
};

template <>
struct Serializer<Type::JSON>
{
    Serializer(const SerializeOptions& options = {}) : options(options), serializer(document.GetAllocator()) {}

    template <typename T>
    Serializer& operator<<(T&& value)
    {
        serializer << std::forward<T>(value);
        return *this;
    }

    std::string& finalize()
    {
        buffer = serializer.finalize(options.compactFrom);
        return buffer;
    }

private:
    std::string buffer;
    SerializeOptions options;
    Impl::JsonSerializer::Document document;
    Impl::JsonSerializer serializer;
};

template <>
struct Serializer<Type::XML>
{
    Serializer(const SerializeOptions& options = {}) : options(options), serializer(document, &document) {}

    template <typename T>
    Serializer& operator<<(T&& value)
    {
        serializer << std::forward<T>(value);
        return *this;
    }

    std::string& finalize()
    {
        buffer = serializer.finalize(options.compactFrom);
        return buffer;
    }

private:
    std::string buffer;
    SerializeOptions options;
    Impl::XmlSerializer::Document document;
    Impl::XmlSerializer serializer;
};

template <Type type>
struct Deserializer
{
};

template <>
struct Deserializer<Type::Binary>
{
    Deserializer(StringView data, const DeserializeOptions& options = {})
    {
        uint8_t flags;
        Size header = 0;

        Impl::readHeader(data, flags, header);
        bool compress = flags & Impl::Flags::Compress;
        bool encrypt = flags & Impl::Flags::Encrypt;

        if (compress && !encrypt)
            Impl::decompress(data.view(header), buffer);
        else if (!compress && encrypt)
            Impl::decrypt(data.view(header), buffer, options.decryptPassword);
        else if (compress && encrypt)
        {
            std::string temp;
            Impl::decrypt(data.view(header), temp, options.decryptPassword);
            Impl::decompress(temp, buffer);
        }
        else
            buffer.assign(data.data + header, data.data + data.size);
    }

    template <typename T>
    Deserializer& operator>>(T&& value)
    {
        StringView temp{buffer.data() + current, Size(buffer.size() - current)};
        current += Impl::deserialize<Binary>({}, temp, std::forward<T>(value));
        return *this;
    }

private:
    std::string buffer;
    Size current = 0;
};

template <>
struct Deserializer<Type::JSON>
{
    Deserializer(const std::string& data, const DeserializeOptions& options = {})
        : deserializer(Impl::JsonDeserializer::parse(document, data))
    {
    }

    template <typename T>
    Deserializer& operator>>(T&& value)
    {
        deserializer >> std::forward<T>(value);
        return *this;
    }

private:
    Impl::JsonDeserializer::Document document;
    Impl::JsonDeserializer deserializer;
};

template <>
struct Deserializer<Type::XML>
{
    Deserializer(const std::string& data, const DeserializeOptions& options = {})
        : deserializer(Impl::XmlDeserializer::parse(document, data))
    {
    }

    template <typename T>
    Deserializer& operator>>(T&& value)
    {
        deserializer >> std::forward<T>(value);
        return *this;
    }

private:
    Impl::XmlDeserializer::Document document;
    Impl::XmlDeserializer deserializer;
};

template <Type type>
struct StreamSerializer
{
};

template <>
struct StreamSerializer<Type::Binary>
{
    StreamSerializer(std::ostream& stream, const SerializeOptions& options = {}, bool multi = true) : stream(stream)
    {
        Impl::writeHeader(stream);
    }

    template <typename T>
    StreamSerializer& operator<<(T&& value)
    {
        Impl::StreamSerializer(stream).process(std::forward<T>(value));
        return *this;
    }

    inline void finalize() {}

private:
    std::ostream& stream;
};

template <>
struct StreamSerializer<Type::JSON>
{
    StreamSerializer(std::ostream& stream, const SerializeOptions& options = {}, bool multi = true)
        : stream(stream), serializer(document.GetAllocator())
    {
        if (multi) serializer.startArray();
    }

    template <typename T>
    StreamSerializer& operator<<(T&& value)
    {
        SERIO_ASSERT(!finalized, "Can't use a finalized stream serializer");
        serializer << std::forward<T>(value);
        return *this;
    }

    void finalize()
    {
        SERIO_ASSERT(!finalized, "Can't use a finalized stream serializer");
        serializer.finalize(stream, options.compactFrom);
        finalized = true;
    }

    ~StreamSerializer() noexcept
    {
        if (!finalized) finalize();
    }

private:
    bool finalized = false;
    std::ostream& stream;
    SerializeOptions options;
    Impl::JsonSerializer::Document document;
    Impl::JsonSerializer serializer;
};

template <>
struct StreamSerializer<Type::XML>
{
    StreamSerializer(std::ostream& stream, const SerializeOptions& options = {}, bool multi = true)
        : stream(stream), serializer(document, &document)
    {
    }

    template <typename T>
    StreamSerializer& operator<<(T&& value)
    {
        SERIO_ASSERT(!finalized, "Can't use a finalized stream serializer");
        serializer << std::forward<T>(value);
        return *this;
    }

    void finalize()
    {
        SERIO_ASSERT(!finalized, "Can't use a finalized stream serializer");
        serializer.finalize(stream, options.compactFrom);
        finalized = true;
    }

    ~StreamSerializer() noexcept
    {
        if (!finalized) finalize();
    }

private:
    bool finalized = false;
    std::ostream& stream;
    SerializeOptions options;
    Impl::XmlSerializer::Document document;
    Impl::XmlSerializer serializer;
};

template <Type type>
struct StreamDeserializer
{
};

template <>
struct StreamDeserializer<Type::Binary>
{
    StreamDeserializer(std::istream& stream, const DeserializeOptions& options = {}, bool multi = true)
        : stream(stream), options(options)
    {
        uint8_t flags;
        Impl::readHeader(stream, flags);
    }

    template <typename T>
    StreamDeserializer& operator>>(T&& value)
    {
        Impl::StreamDeserializer(stream, options.maxLength).process(std::forward<T>(value));
        return *this;
    }

private:
    std::istream& stream;
    DeserializeOptions options;
};

template <>
struct StreamDeserializer<Type::JSON>
{
    StreamDeserializer(std::istream& stream, const DeserializeOptions& options = {}, bool multi = true)
        : deserializer(Impl::JsonDeserializer::parse(document, stream))
    {
        if (multi) deserializer.startArray();
    }

    template <typename T>
    StreamDeserializer& operator>>(T&& value)
    {
        deserializer >> std::forward<T>(value);
        return *this;
    }

private:
    Impl::JsonDeserializer::Document document;
    Impl::JsonDeserializer deserializer;
};

template <>
struct StreamDeserializer<Type::XML>
{
    StreamDeserializer(std::istream& stream, const DeserializeOptions& options = {}, bool multi = true)
        : buffer(), deserializer(Impl::XmlDeserializer::parse(document, stream, buffer))
    {
    }

    template <typename T>
    StreamDeserializer& operator>>(T&& value)
    {
        deserializer >> std::forward<T>(value);
        return *this;
    }

private:
    std::string buffer;
    Impl::XmlDeserializer::Document document;
    Impl::XmlDeserializer deserializer;
};
}  // namespace Serio

namespace std
{
// Ostream overload for std::pair
template <class F, class S>
inline ::std::ostream& operator<<(::std::ostream& os, ::std::pair<F, S> const& p)
{
    os << "([" << p.first << "], [" << p.second << "])";
    return os;
}
}  // namespace std

// Checks that collections have equal size and all elements are the same
template <class T>
inline void check_collection(T const& a, T const& b)
{
    auto aIter = std::begin(a);
    auto aEnd = std::end(a);
    auto bIter = std::begin(b);
    auto bEnd = std::end(b);

    CHECK_EQ(std::distance(aIter, aEnd), std::distance(bIter, bEnd));

    for (; aIter != aEnd; ++aIter, ++bIter) CHECK_EQ(*aIter, *bIter);
}

template <class T>
inline void check_ptr_collection(T const& a, T const& b)
{
    auto aIter = std::begin(a);
    auto aEnd = std::end(a);
    auto bIter = std::begin(b);
    auto bEnd = std::end(b);

    CHECK_EQ(std::distance(aIter, aEnd), std::distance(bIter, bEnd));

    for (; aIter != aEnd; ++aIter, ++bIter) CHECK_EQ(**aIter, **bIter);
}

// Random Number Generation ===============================================
template <class T>
inline typename std::enable_if<std::is_floating_point<T>::value, T>::type random_value(std::mt19937& gen)
{
    return std::uniform_real_distribution<T>(-10000.0, 10000.0)(gen);
}

template <class T>
inline typename std::enable_if<std::is_integral<T>::value && sizeof(T) != sizeof(char), T>::type random_value(
    std::mt19937& gen)
{
    return std::uniform_int_distribution<T>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max())(gen);
}

template <class T>
inline typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(char), T>::type random_value(
    std::mt19937& gen)
{
    return static_cast<T>(
        std::uniform_int_distribution<int64_t>(std::numeric_limits<T>::lowest(), std::numeric_limits<T>::max())(gen));
}

template <class T>
inline typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type random_value(std::mt19937& gen)
{
    std::string s(std::uniform_int_distribution<int>(3, 30)(gen), ' ');
    for (char& c : s) c = static_cast<char>(std::uniform_int_distribution<int>('A', 'Z')(gen));
    return s;
}

inline size_t random_index(size_t min, size_t max, std::mt19937& gen)
{
    return std::uniform_int_distribution<size_t>(min, max)(gen);
}

template <class C>
inline std::basic_string<C> random_basic_string(std::mt19937& gen)
{
    std::basic_string<C> s(std::uniform_int_distribution<int>(3, 30)(gen), ' ');
    for (C& c : s) c = static_cast<C>(std::uniform_int_distribution<int>('A', 'Z')(gen));
    return s;
}

template <size_t N>
inline std::string random_binary_string(std::mt19937& gen)
{
    std::string s(N, ' ');
    for (auto& c : s) c = static_cast<char>(std::uniform_int_distribution<int>('0', '1')(gen));
    return s;
}

// Generic struct useful for testing many serialization functions
struct StructBase
{
    SERIO_REGISTER(x, y)

    StructBase() {}
    StructBase(int xx, int yy) : x(xx), y(yy) {}
    int x, y;
    bool operator==(StructBase const& other) const { return x == other.x && y == other.y; }
    bool operator!=(StructBase const& other) const { return x != other.x || y != other.y; }
    bool operator<(StructBase const& other) const
    {
        if (x < other.x)
            return true;
        else if (other.x < x)
            return false;
        else
            return (y < other.y);
    }
};

inline std::ostream& operator<<(std::ostream& os, StructBase const& s)
{
    os << "[x: " << s.x << " y: " << s.y << "]";
    return os;
}

struct StructInternalSerialize : StructBase
{
    StructInternalSerialize() : StructBase{0, 0} {}
    StructInternalSerialize(int x_, int y_) : StructBase{x_, y_} {}
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(x, y);
    }
};

struct StructInternalSplit : StructBase
{
    StructInternalSplit() : StructBase{0, 0} {}
    StructInternalSplit(int x_, int y_) : StructBase{x_, y_} {}
    template <class Archive>
    void save(Archive& ar) const
    {
        ar(x, y);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        ar(x, y);
    }
};

struct StructExternalSerialize : StructBase
{
    StructExternalSerialize() : StructBase{0, 0} {}
    StructExternalSerialize(int x_, int y_) : StructBase{x_, y_} {}
};

template <class Archive>
void serialize(Archive& ar, StructExternalSerialize& s)
{
    ar(s.x, s.y);
}

struct StructExternalSplit : StructBase
{
    StructExternalSplit() : StructBase{0, 0} {}
    StructExternalSplit(int x_, int y_) : StructBase{x_, y_} {}
};

template <class Archive>
inline void save(Archive& ar, StructExternalSplit const& s)
{
    ar(s.x, s.y);
}

template <class Archive>
inline void load(Archive& ar, StructExternalSplit& s)
{
    ar(s.x, s.y);
}

template <class T>
struct StructHash
{
public:
    size_t operator()(const T& s) const
    {
        size_t h1 = std::hash<int>()(s.x);
        size_t h2 = std::hash<int>()(s.y);
        return h1 ^ (h2 << 1);
    }
};

template <Serio::Type T>
struct TestSerializer
{
    Serio::StreamSerializer<T> stream;

    TestSerializer(std::ostream& stream) : stream(stream) {}

    TestSerializer& operator()() { return *this; }

    template <typename Head, typename... Tail>
    TestSerializer& operator()(Head&& head, Tail&&... tail)
    {
        stream << std::forward<Head>(head);
        (*this)(std::forward<Tail>(tail)...);
        return *this;
    }
};

template <Serio::Type T>
struct TestDeserializer
{
    Serio::StreamDeserializer<T> stream;

    TestDeserializer(std::istream& stream) : stream(stream) {}

    TestDeserializer& operator()() { return *this; }

    template <typename Head, typename... Tail>
    TestDeserializer& operator()(Head&& head, Tail&&... tail)
    {
        stream >> std::forward<Head>(head);
        (*this)(std::forward<Tail>(tail)...);
        return *this;
    }
};

template <typename T, typename C>
C& TestContainer(std::stack<T, C>& value)
{
    return (C&)value;
}

template <typename T, typename C>
C& TestContainer(std::queue<T, C>& value)
{
    return (C&)value;
}

template <typename T, typename C>
C& TestContainer(std::priority_queue<T, C>& value)
{
    return (C&)value;
}
