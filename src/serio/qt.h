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

#include <serio/serio.h>

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QMap>
#include <QMultiHash>
#include <QMultiMap>
#include <QQueue>
#include <QSet>
#include <QStack>
#include <QString>
#include <QStringList>
#include <QVarLengthArray>
#include <QVector>

namespace Serio
{
namespace Impl
{
template <typename T>
struct IsResizable<QVector<T>> : std::true_type
{
};
template <>
struct IsResizable<QByteArray> : std::true_type
{
};
template <>
struct IsResizable<QString> : std::true_type
{
};
template <typename T, qsizetype N>
struct IsResizable<QVarLengthArray<T, N>> : std::true_type
{
};
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template <typename T>
struct IsResizable<QStack<T>> : std::true_type
{
};
#endif

template <typename T>
struct IsQtAppendable : std::false_type
{
};
template <>
struct IsQtAppendable<QStringList> : std::true_type
{
};
template <typename T>
struct IsQtAppendable<QList<T>> : std::true_type
{
};
template <typename T>
struct IsQtAppendable<QQueue<T>> : std::true_type
{
};
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
template <typename T>
struct IsQtAppendable<QStack<T>> : std::true_type
{
};
#endif

template <typename T>
struct IsQtMap : std::false_type
{
};
template <typename K, typename V>
struct IsQtMap<QMap<K, V>> : std::true_type
{
};
template <typename K, typename V>
struct IsQtMap<QHash<K, V>> : std::true_type
{
};
template <typename K, typename V>
struct IsQtMap<QMultiMap<K, V>> : std::true_type
{
};
template <typename K, typename V>
struct IsQtMap<QMultiHash<K, V>> : std::true_type
{
};
// Use upperBound hint for sorted Qt maps (QMap/QMultiMap); fall back for hash maps (no upperBound).
template <typename Map, typename K, typename V>
auto qtInsert(Map& m, const K& key, V&& val, int) -> decltype(m.upperBound(key), void())
{
    m.insert(m.upperBound(key), key, std::forward<V>(val));
}
template <typename Map, typename K, typename V>
void qtInsert(Map& m, const K& key, V&& val, long)
{
    m.insert(key, std::forward<V>(val));
}
}  // namespace Impl

template <>
struct CustomClass<QChar>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const QChar& value, Serializer& C)
    {
        C << value.unicode();
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(QChar& value, Deserializer& C)
    {
        value = QChar(C.template get<char16_t>());
    }

    template <typename Serializer>
    IfJSON<Serializer> serialize(const QChar& value, Serializer& C)
    {
        C << value.unicode();
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(QChar& value, Deserializer& C)
    {
        value = QChar(C.template get<char16_t>());
    }
};

template <typename T>
struct CustomClass<QSet<T>>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const QSet<T>& value, Serializer& C)
    {
        C << Size(value.size());
        for (const auto& item : value) C << item;
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(QSet<T>& value, Deserializer& C)
    {
        value.clear();
        auto size = C.getLength();
        for (Size i = 0; i < size; ++i) value.insert(C.template get<T>());
    }

    template <typename Serializer>
    IfJSON<Serializer> serialize(const QSet<T>& value, Serializer& C)
    {
        C.startArray();
        for (const auto& item : value) C << item;
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(QSet<T>& value, Deserializer& C)
    {
        value.clear();
        auto size = C.startArray();
        for (Size i = 0; i < size; ++i) value.insert(C.template get<T>());
    }
};

template <typename T>
struct CustomClass<T, typename std::enable_if<Impl::IsQtAppendable<T>::value>::type>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const T& value, Serializer& C)
    {
        C << Size(value.size());
        for (const auto& item : value) C << item;
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(T& value, Deserializer& C)
    {
        using V = typename T::value_type;

        value.clear();
        auto size = C.getLength();
        for (Size i = 0; i < size; ++i) value.append(C.template get<V>());
    }

    template <typename Serializer>
    IfJSON<Serializer> serialize(const T& value, Serializer& C)
    {
        C.startArray();
        for (const auto& item : value) C << item;
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(T& value, Deserializer& C)
    {
        using V = typename T::value_type;

        value.clear();
        auto size = C.startArray();
        for (Size i = 0; i < size; ++i) value.append(C.template get<V>());
    }
};

template <typename T>
struct CustomClass<T, typename std::enable_if<Impl::IsQtMap<T>::value>::type>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const T& value, Serializer& C)
    {
        C << Size(value.size());
        for (auto i = value.cbegin(), end = value.cend(); i != end; ++i) C << i.key() << i.value();
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(T& value, Deserializer& C)
    {
        using K = typename T::key_type;
        using V = typename T::mapped_type;

        value.clear();
        auto size = C.getLength();
        for (Size i = 0; i < size; ++i)
        {
            auto A = C.template get<K>();
            auto B = C.template get<V>();
            Impl::qtInsert(value, A, std::move(B), 0);
        }
    }

    template <typename Serializer>
    IfJSON<Serializer> serialize(const T& value, Serializer& C)
    {
        C.startArray();
        for (auto i = value.cbegin(), end = value.cend(); i != end; ++i)
        {
            const auto& key = i.key();
            const auto& val = i.value();
            C << std::tie(key, val);
        }
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(T& value, Deserializer& C)
    {
        using K = typename T::key_type;
        using V = typename T::mapped_type;

        value.clear();
        auto size = C.startArray();
        for (Size i = 0; i < size; ++i)
        {
            auto pair = C.template get<std::tuple<K, V>>();
            auto& key = std::get<0>(pair);
            Impl::qtInsert(value, key, std::move(std::get<1>(pair)), 0);
        }
    }
};
}  // namespace Serio
