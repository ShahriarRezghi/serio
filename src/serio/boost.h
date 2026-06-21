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

#include <boost/array.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/devector.hpp>
#include <boost/container/list.hpp>
#include <boost/container/slist.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>
//
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/map.hpp>
#include <boost/container/set.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
//
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/variant.hpp>
//
#include <boost/fusion/container/deque/back_extended_deque.hpp>
#include <boost/fusion/container/deque/front_extended_deque.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/cons.hpp>
#include <boost/fusion/include/deque.hpp>
#include <boost/fusion/include/list.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/nil.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/set.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/tuple.hpp>

namespace Serio
{
namespace Impl
{
using N = boost::tuples::null_type;

template <typename T>
struct BoostTupleSize
{
};
template <typename... Ts>
struct BoostTupleSize<boost::tuple<Ts...>>
{
    constexpr static size_t size() { return 10; }
};
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8,
          typename A9>
struct BoostTupleSize<boost::tuple<A1, A2, A3, A4, A5, A6, A7, A8, A9, N>>
{
    constexpr static size_t size() { return 9; }
};
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
struct BoostTupleSize<boost::tuple<A1, A2, A3, A4, A5, A6, A7, A8, N, N>>
{
    constexpr static size_t size() { return 8; }
};
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
struct BoostTupleSize<boost::tuple<A1, A2, A3, A4, A5, A6, A7, N, N, N>>
{
    constexpr static size_t size() { return 7; }
};
template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct BoostTupleSize<boost::tuple<A1, A2, A3, A4, A5, A6, N, N, N, N>>
{
    constexpr static size_t size() { return 6; }
};
template <typename A1, typename A2, typename A3, typename A4, typename A5>
struct BoostTupleSize<boost::tuple<A1, A2, A3, A4, A5, N, N, N, N, N>>
{
    constexpr static size_t size() { return 5; }
};
template <typename A1, typename A2, typename A3, typename A4>
struct BoostTupleSize<boost::tuple<A1, A2, A3, A4, N, N, N, N, N, N>>
{
    constexpr static size_t size() { return 4; }
};
template <typename A1, typename A2, typename A3>
struct BoostTupleSize<boost::tuple<A1, A2, A3, N, N, N, N, N, N, N>>
{
    constexpr static size_t size() { return 3; }
};
template <typename A1, typename A2>
struct BoostTupleSize<boost::tuple<A1, A2, N, N, N, N, N, N, N, N>>
{
    constexpr static size_t size() { return 2; }
};
template <typename A1>
struct BoostTupleSize<boost::tuple<A1, N, N, N, N, N, N, N, N, N>>
{
    constexpr static size_t size() { return 1; }
};
template <>
struct BoostTupleSize<boost::tuple<N, N, N, N, N, N, N, N, N, N>>
{
    constexpr static size_t size() { return 0; }
};

template <size_t I>
struct BoostTuple
{
    template <typename Serializer, typename T>
    static void serialize(Serializer& item, const T& value)
    {
        BoostTuple<I - 1>::serialize(item, value);
        item << boost::get<I>(value);
    }
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, T& value)
    {
        BoostTuple<I - 1>::deserialize(item, value);
        item >> boost::get<I>(value);
    }
};
template <>
struct BoostTuple<0>
{
    template <typename Serializer, typename T>
    static void serialize(Serializer& item, const T& value)
    {
        item << boost::get<0>(value);
    }
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, T& value)
    {
        item >> boost::get<0>(value);
    }
};

template <size_t I>
struct FusionSet
{
    template <typename Serializer, typename... Ts>
    static void serialize(Serializer& item, const boost::fusion::set<Ts...>& value)
    {
        FusionSet<I - 1>::serialize(item, value);
        using T = typename std::tuple_element<I, std::tuple<Ts...>>::type;
        item << boost::fusion::at_key<T>(value);
    }
    template <typename Deserializer, typename... Ts>
    static void deserialize(Deserializer& item, boost::fusion::set<Ts...>& value)
    {
        FusionSet<I - 1>::deserialize(item, value);
        using T = typename std::tuple_element<I, std::tuple<Ts...>>::type;
        item >> boost::fusion::at_key<T>(value);
    }
};
template <>
struct FusionSet<0>
{
    template <typename Serializer, typename... Ts>
    static void serialize(Serializer& item, const boost::fusion::set<Ts...>& value)
    {
        using T = typename std::tuple_element<0, std::tuple<Ts...>>::type;
        item << boost::fusion::at_key<T>(value);
    }
    template <typename Deserializer, typename... Ts>
    static void deserialize(Deserializer& item, boost::fusion::set<Ts...>& value)
    {
        using T = typename std::tuple_element<0, std::tuple<Ts...>>::type;
        item >> boost::fusion::at_key<T>(value);
    }
};

template <size_t I>
struct FusionMap
{
    template <typename Serializer, typename... Ts>
    static void serialize(Serializer& item, const boost::fusion::map<Ts...>& value)
    {
        FusionMap<I - 1>::serialize(item, value);
        using T = typename std::tuple_element<I, std::tuple<Ts...>>::type::first_type;
        item << boost::fusion::at_key<T>(value);
    }
    template <typename Deserializer, typename... Ts>
    static void deserialize(Deserializer& item, boost::fusion::map<Ts...>& value)
    {
        FusionMap<I - 1>::deserialize(item, value);
        using T = typename std::tuple_element<I, std::tuple<Ts...>>::type::first_type;
        item >> boost::fusion::at_key<T>(value);
    }
};
template <>
struct FusionMap<0>
{
    template <typename Serializer, typename... Ts>
    static void serialize(Serializer& item, const boost::fusion::map<Ts...>& value)
    {
        using T = typename std::tuple_element<0, std::tuple<Ts...>>::type::first_type;
        item << boost::fusion::at_key<T>(value);
    }
    template <typename Deserializer, typename... Ts>
    static void deserialize(Deserializer& item, boost::fusion::map<Ts...>& value)
    {
        using T = typename std::tuple_element<0, std::tuple<Ts...>>::type::first_type;
        item >> boost::fusion::at_key<T>(value);
    }
};

template <typename T, typename... Ts>
struct IsContinuous<boost::container::basic_string<T, Ts...>> : std::is_arithmetic<T>
{
};
template <typename T, typename... Ts>
struct IsContinuous<boost::container::vector<T, Ts...>> : std::is_arithmetic<T>
{
};
template <typename T, size_t N, typename... Ts>
struct IsContinuous<boost::container::static_vector<T, N, Ts...>> : std::is_arithmetic<T>
{
};
template <typename T, size_t N, typename... Ts>
struct IsContinuous<boost::container::small_vector<T, N, Ts...>> : std::is_arithmetic<T>
{
};
template <typename T, typename... Ts>
struct IsContinuous<boost::container::devector<T, Ts...>> : std::is_arithmetic<T>
{
};
template <typename T, size_t N>
struct IsContinuous<boost::array<T, N>> : std::is_arithmetic<T>
{
};

template <typename T, size_t N>
struct IsFixed<boost::array<T, N>> : std::true_type
{
    static constexpr size_t size() { return N; }
};
template <typename T, size_t N>
struct IsFixedContinuous<boost::array<T, N>> : std::is_arithmetic<T>
{
};

template <typename... Ts>
struct IsResizable<boost::array<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<boost::container::vector<Ts...>> : std::true_type
{
};
template <typename T, size_t N, typename... Ts>
struct IsResizable<boost::container::static_vector<T, N, Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<boost::container::stable_vector<Ts...>> : std::true_type
{
};
template <typename T, size_t N, typename... Ts>
struct IsResizable<boost::container::small_vector<T, N, Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<boost::container::devector<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<boost::container::list<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<boost::container::slist<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<boost::container::deque<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<boost::container::basic_string<Ts...>> : std::true_type
{
};

template <typename... Ts>
struct IsAppendable<boost::container::set<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::container::map<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::container::multiset<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::container::multimap<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::container::flat_set<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::container::flat_map<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::container::flat_multiset<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::container::flat_multimap<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::unordered_set<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::unordered_map<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::unordered_multiset<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<boost::unordered_multimap<Ts...>> : std::true_type
{
};

template <typename... Ts>
struct IsOptional<boost::optional<Ts...>> : std::true_type
{
};

template <typename T>
struct IsBoostTuple : std::false_type
{
    using Size = void;
};
template <typename... Ts>
struct IsBoostTuple<boost::tuple<Ts...>> : std::true_type
{
    using Size = std::tuple_size<std::tuple<Ts...>>;
};
template <typename... Ts>
struct IsBoostTuple<boost::fusion::tuple<Ts...>> : std::true_type
{
    using Size = std::tuple_size<std::tuple<Ts...>>;
};
template <typename... Ts>
struct IsBoostTuple<boost::fusion::vector<Ts...>> : std::true_type
{
    using Size = std::tuple_size<std::tuple<Ts...>>;
};
template <typename... Ts>
struct IsBoostTuple<boost::fusion::list<Ts...>> : std::true_type
{
    using Size = std::tuple_size<std::tuple<Ts...>>;
};
template <typename... Ts>
struct IsBoostTuple<boost::fusion::deque<Ts...>> : std::true_type
{
    using Size = std::tuple_size<std::tuple<Ts...>>;
};
template <typename... Ts>
struct IsBoostTuple<boost::fusion::front_extended_deque<Ts...>> : std::true_type
{
    using Size = std::tuple_size<std::tuple<Ts...>>;
};
template <typename... Ts>
struct IsBoostTuple<boost::fusion::back_extended_deque<Ts...>> : std::true_type
{
    using Size = std::tuple_size<std::tuple<Ts...>>;
};

template <typename K, typename T, typename... Ts>
struct ValueType<boost::container::map<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<boost::container::multimap<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<boost::container::flat_map<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<boost::container::flat_multimap<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<boost::unordered_map<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<boost::unordered_multimap<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
}  // namespace Impl

template <>
struct CustomClass<boost::fusion::nil>
{
    template <typename Serializer>
    void serialize(const boost::fusion::nil& value, Serializer& C)
    {
    }
    template <typename Deserializer>
    void deserialize(boost::fusion::nil& value, Deserializer& C)
    {
    }
};

template <typename... Ts>
struct CustomClass<boost::fusion::pair<Ts...>>
{
    template <typename Serializer>
    void serialize(const boost::fusion::pair<Ts...>& value, Serializer& C)
    {
        C << value.second;
    }
    template <typename Deserializer>
    void deserialize(boost::fusion::pair<Ts...>& value, Deserializer& C)
    {
        C >> value.second;
    }
};

template <typename... Ts>
struct CustomClass<boost::fusion::set<Ts...>>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const boost::fusion::set<Ts...>& value, Serializer& C)
    {
        Impl::FusionSet<sizeof...(Ts) - 1>::serialize(C, value);
    }
    template <typename Serializer>
    IfJSON<Serializer> serialize(const boost::fusion::set<Ts...>& value, Serializer& C)
    {
        C.startArray();
        Impl::FusionSet<sizeof...(Ts) - 1>::serialize(C, value);
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(boost::fusion::set<Ts...>& value, Deserializer& C)
    {
        Impl::FusionSet<sizeof...(Ts) - 1>::deserialize(C, value);
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(boost::fusion::set<Ts...>& value, Deserializer& C)
    {
        C.startTuple(sizeof...(Ts));
        Impl::FusionSet<sizeof...(Ts) - 1>::deserialize(C, value);
    }
};

template <typename... Ts>
struct CustomClass<boost::fusion::map<Ts...>>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const boost::fusion::map<Ts...>& value, Serializer& C)
    {
        Impl::FusionMap<sizeof...(Ts) - 1>::serialize(C, value);
    }
    template <typename Serializer>
    IfJSON<Serializer> serialize(const boost::fusion::map<Ts...>& value, Serializer& C)
    {
        C.startArray();
        Impl::FusionMap<sizeof...(Ts) - 1>::serialize(C, value);
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(boost::fusion::map<Ts...>& value, Deserializer& C)
    {
        Impl::FusionMap<sizeof...(Ts) - 1>::deserialize(C, value);
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(boost::fusion::map<Ts...>& value, Deserializer& C)
    {
        C.startTuple(sizeof...(Ts));
        Impl::FusionMap<sizeof...(Ts) - 1>::deserialize(C, value);
    }
};

// Base case: cons whose tail is nil (single-element cons)
template <typename H>
struct CustomClass<boost::fusion::cons<H, boost::fusion::nil_>>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const boost::fusion::cons<H, boost::fusion::nil_>& value, Serializer& C)
    {
        C << value.car;
    }
    template <typename Serializer>
    IfJSON<Serializer> serialize(const boost::fusion::cons<H, boost::fusion::nil_>& value, Serializer& C)
    {
        C.startArray();
        C << value.car;
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(boost::fusion::cons<H, boost::fusion::nil_>& value, Deserializer& C)
    {
        C >> value.car;
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(boost::fusion::cons<H, boost::fusion::nil_>& value, Deserializer& C)
    {
        C.startTuple(1);
        C >> value.car;
    }
};

// Recursive case: cons with a non-nil tail (multi-element cons)
template <typename H, typename T>
struct CustomClass<boost::fusion::cons<H, T>>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const boost::fusion::cons<H, T>& value, Serializer& C)
    {
        C << value.car << value.cdr;
    }
    template <typename Serializer>
    IfJSON<Serializer> serialize(const boost::fusion::cons<H, T>& value, Serializer& C)
    {
        C.startArray();
        C << value.car << value.cdr;
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(boost::fusion::cons<H, T>& value, Deserializer& C)
    {
        C >> value.car >> value.cdr;
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(boost::fusion::cons<H, T>& value, Deserializer& C)
    {
        C.startTuple(2);
        C >> value.car >> value.cdr;
    }
};

template <typename... Ts>
struct CustomClass<boost::variant<Ts...>>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const boost::variant<Ts...>& value, Serializer& C)
    {
        C << Size(value.which());
        boost::apply_visitor([&](auto&& arg) { C << arg; }, value);
    }
    template <typename Serializer>
    IfJSON<Serializer> serialize(const boost::variant<Ts...>& value, Serializer& C)
    {
        C.startArray();
        C << Size(value.which());
        boost::apply_visitor([&](auto&& arg) { C << arg; }, value);
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(boost::variant<Ts...>& value, Deserializer& C)
    {
        auto index = C.template get<Size>();
        Impl::Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(C, index, value);
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(boost::variant<Ts...>& value, Deserializer& C)
    {
        C.startTuple(2);
        auto index = C.template get<Size>();
        Impl::Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(C, index, value);
    }
};

template <typename... Ts>
struct CustomClass<boost::tuple<Ts...>>
{
    template <typename Serializer>
    IfNotJSON<Serializer> serialize(const boost::tuple<Ts...>& value, Serializer& C)
    {
        using TP = boost::tuple<Ts...>;
        Impl::BoostTuple<Impl::BoostTupleSize<TP>::size() - 1>::serialize(C, value);
    }
    template <typename Serializer>
    IfJSON<Serializer> serialize(const boost::tuple<Ts...>& value, Serializer& C)
    {
        using TP = boost::tuple<Ts...>;
        C.startArray();
        Impl::BoostTuple<Impl::BoostTupleSize<TP>::size() - 1>::serialize(C, value);
    }
    template <typename Deserializer>
    IfNotJSON<Deserializer> deserialize(boost::tuple<Ts...>& value, Deserializer& C)
    {
        using TP = boost::tuple<Ts...>;
        Impl::BoostTuple<Impl::BoostTupleSize<TP>::size() - 1>::deserialize(C, value);
    }
    template <typename Deserializer>
    IfJSON<Deserializer> deserialize(boost::tuple<Ts...>& value, Deserializer& C)
    {
        using TP = boost::tuple<Ts...>;
        C.startTuple(Impl::BoostTupleSize<TP>::size());
        Impl::BoostTuple<Impl::BoostTupleSize<TP>::size() - 1>::deserialize(C, value);
    }
};
}  // namespace Serio
