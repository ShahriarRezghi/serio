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

#include <serio/binary.h>

#include <iomanip>
#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_print.hpp>
#include <sstream>

#include "serio/base.h"

namespace Serio
{
namespace Impl
{
struct XmlSerializer : public Base::Serializer, public Base::XML
{
    using Derived = XmlSerializer;

    using Document = rapidxml::xml_document<char>;
    using Node = rapidxml::xml_node<char>*;
    using Attr = rapidxml::xml_attribute<char>*;
    using Type = rapidxml::node_type;

    Document& document;

    Node createNode()
    {
        auto node = document.allocate_node(Type::node_element);
        node->name("item");
        return node;
    }
    Node createString(const std::string& value)
    {
        auto node = createNode();
        auto ptr = document.allocate_string(value.data(), value.size());
        node->value(ptr, value.size());
        return node;
    }
    Node createNull() { return createNode(); }

    void addAttribute(Node node, const std::string& name, const std::string& value)
    {
        auto namePtr = document.allocate_string(name.data(), name.size());
        auto valuePtr = document.allocate_string(value.data(), value.size());
        auto attr = document.allocate_attribute(namePtr, valuePtr, name.size(), value.size());
        node->append_attribute(attr);
    }

    template <typename T>
    EnableIfT<std::is_class<T>::value && !IsFixed<T>::value && !IsResizable<T>::value && !IsAppendable<T>::value &&
                  !IsPointer<T>::value && !IsOptional<T>::value,
              Node>
    serialize(const T& value)
    {
        Derived serializer(document, createNode());
        CustomClass<T>().serialize(value, serializer);
        return serializer.xml;
    }
    template <typename T>
    EnableIfT<IsFixed<T>::value || IsResizable<T>::value || IsAppendable<T>::value, Node> serialize(const T& value)
    {
        auto node = createNode();
        for (const auto& item : value) node->append_node(serialize(item));
        return node;
    }
    template <typename T>
    EnableIfT<IsPointer<T>::value, Node> serialize(const T& value)
    {
        auto node = createNode();
        addAttribute(node, "valid", value ? "true" : "false");
        node->append_node(value ? serialize(*value.get()) : createNull());
        return node;
    }
    template <typename... Ts>
    Node serialize(const std::chrono::duration<Ts...>& value)
    {
        return serialize(value.count());
    }
    template <typename... Ts>
    Node serialize(const std::chrono::time_point<Ts...>& value)
    {
        return serialize(value.time_since_epoch());
    }

    template <typename T>
    Node serialize(const std::vector<bool, T>& value)
    {
        auto node = createNode();
        for (const auto& item : value) node->append_node(serialize(bool(item)));
        return node;
    }
    template <typename T>
    EnableIfT<std::is_integral<T>::value || std::is_enum<T>::value, Node> serialize(const T& value)
    {
        return createString(std::to_string(int64_t(value)));
    }
    template <typename T>
    EnableIfT<std::is_floating_point<T>::value, Node> serialize(const T& value)
    {
        std::ostringstream oss;
        oss << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
        return createString(oss.str());
    }
    inline Node serialize(const uint64_t& value)  //
    {
        return createString(std::to_string(value));
    }
    inline Node serialize(const bool& value)  //
    {
        return createString(value ? "true" : "false");
    }
    template <typename... Ts>
    Node serialize(const std::basic_string<char, Ts...>& value)
    {
        return createString(value);
    }
    template <typename... Ts>
    Node serialize(const std::basic_string<Ts...>& value)
    {
        return serialize(toutf8(value.data(), value.data() + value.size()));
    }
    template <size_t N>
    Node serialize(const std::bitset<N>& value)
    {
        return createString(value.to_string());
    }
    template <typename... Ts>
    Node serialize(const std::queue<Ts...>& value)
    {
        return serialize(static_cast<const Queue<Ts...>&>(value).c);
    }
    template <typename... Ts>
    Node serialize(const std::stack<Ts...>& value)
    {
        return serialize(static_cast<const Stack<Ts...>&>(value).c);
    }
    template <typename... Ts>
    Node serialize(const std::priority_queue<Ts...>& value)
    {
        return serialize(static_cast<const PQueue<Ts...>&>(value).c);
    }
    template <typename T1, typename T2>
    Node serialize(const std::pair<T1, T2>& value)
    {
        auto node = createNode();
        node->append_node(serialize(value.first));
        node->append_node(serialize(value.second));
        return node;
    }
    template <typename... Ts>
    Node serialize(const std::tuple<Ts...>& value)
    {
        Derived serializer(document, createNode());
        Tuple<sizeof...(Ts)>::serialize(serializer, value);
        return serializer.xml;
    }
    template <typename T>
    Node serialize(const std::complex<T>& value)
    {
        auto node = createNode();
        node->append_node(serialize(value.real()));
        node->append_node(serialize(value.imag()));
        return node;
    }
    template <typename... Ts>
    Node serialize(const std::atomic<Ts...>& value)
    {
        return serialize(value.load());
    }
    template <typename T>
    Node serialize(const BinaryString<T>& value)
    {
        return serializeBinary(value.value.data(), value.value.size());
    }
    template <typename... Ts>
    Node serializeBinary(const char* data, size_t insize)
    {
        auto outsize = b64encSize(insize);
        auto output = document.allocate_string(outsize ? 0 : "", outsize);
        base64Encode((const uint8_t*)data, output, insize);

        auto node = createNode();
        node->value(output, outsize);
        return node;
    }
    template <typename T>
    Node serializeBinary(const T* data, size_t size)
    {
        auto str = toutf8(data, data + size);
        return serializeBinary(str.data(), str.size());
    }

#if SERIO_CPP_VERSION >= 201703L
    template <typename... Ts>
    Node serialize(const std::basic_string_view<Ts...>& value)
    {
        return serialize(toutf8(value.data(), value.data() + value.size()));
    }
    template <typename T>
    EnableIfT<IsOptional<T>::value, Node> serialize(const T& value)
    {
        auto node = createNode();
        addAttribute(node, "valid", value ? "true" : "false");
        node->append_node(value ? serialize(value.value()) : createNull());
        return node;
    }
    template <typename... Ts>
    Node serialize(const std::variant<Ts...>& value)
    {
        auto node = createNode();
        node->append_node(serialize(value.index()));
        std::visit([&](auto&& arg) { node->append_node(serialize(arg)); }, value);
        return node;
    }
    inline Node serialize(const std::monostate&) { return createNull(); }
#endif

#if SERIO_CPP_VERSION >= 202302L
    template <typename... Ts>
    Node serialize(const std::expected<Ts...>& value)
    {
        auto node = createNode();
        addAttribute(node, "valid", value ? "true" : "false");
        node->append_node(value ? serialize(value.value()) : serialize(value.error()));
        return node;
    }
#endif

#if SERIO_ENABLE_FILESYSTEM
    Node serialize(const std::filesystem::path& value)  //
    {
        return serialize(binaryString(value.string()));
    }
#endif

public:
    Node xml;

    inline XmlSerializer(Document& document, Node xml)  //
        : document(document), xml(xml)
    {
    }

    template <typename T>
    Derived& operator<<(const T& value)
    {
        xml->append_node(serialize(value));
        return *this;
    }
    template <typename T>
    Derived& operator<<(const NVP<T>& value)
    {
        auto node = serialize(value.value);
        auto namePtr = document.allocate_string(value.name.data, value.name.size);
        node->name(namePtr, value.name.size);
        xml->append_node(node);
        return *this;
    }

    template <typename Head, typename... Tail>
    Derived& process(Head&& head, Tail&&... tail)
    {
        *this << std::forward<Head>(head);
        return process(std::forward<Tail>(tail)...);
    }
    Derived& process() { return *this; }

    std::string finalize(bool compact)
    {
        std::stringstream stream;
        std::ostream_iterator<std::string::value_type> iter(stream);
        rapidxml::print(iter, document, compact ? rapidxml::print_no_indenting : 0);
        return stream.str();
    }

    void finalize(std::ostream& stream, bool compact)
    {
        std::ostream_iterator<std::string::value_type> iter(stream);
        rapidxml::print(iter, document, compact ? rapidxml::print_no_indenting : 0);
    }
};

struct XmlDeserializer : public Base::Deserializer, public Base::XML
{
    using Derived = XmlDeserializer;

    using Document = rapidxml::xml_document<char>;
    using Node = rapidxml::xml_node<char>*;
    using Attr = rapidxml::xml_attribute<char>*;
    using Type = rapidxml::node_type;

    Node node;

    template <typename T>
    T iget(Node item)
    {
        T value;
        deserialize(item, value);
        return value;
    }

    std::string getString(Node item)  //
    {
        // TODO trim?
        return std::string(item->value(), item->value_size());
    }
    std::vector<Node> getChildren(Node item)
    {
        std::vector<Node> list;
        for (auto n = item->first_node(); n; n = n->next_sibling()) list.push_back(n);
        return list;
    }
    std::string getAttribute(Node item, const std::string& name)
    {
        auto attr = item->first_attribute(name.data(), name.size());
        SERIO_ASSERT(attr, "Failed to find attribute " + name + " in xml node");
        return std::string(attr->value(), attr->value_size());
    }

    // TODO remove
    bool isNull(Node item) { return getString(item).empty() && item->first_node() == nullptr; }

    template <typename T>
    EnableIfT<std::is_class<T>::value && !IsFixed<T>::value && !IsResizable<T>::value && !IsAppendable<T>::value &&
                  !IsPointer<T>::value && !IsOptional<T>::value,
              void>
    deserialize(Node item, T& value)
    {
        Derived deserializer(item->first_node());
        CustomClass<T>().deserialize(value, deserializer);
    }
    template <typename T>
    EnableIfT<IsFixed<T>::value, void> deserialize(Node item, T& value)
    {
        auto nodes = getChildren(item);
        SERIO_ASSERT(nodes.size() == IsFixed<T>::size(), "Invalid array size encountered");
        for (size_t i = 0; i < nodes.size(); ++i) deserialize(nodes[i], value[i]);
    }
    template <typename T>
    EnableIfT<IsAppendable<T>::value, void> deserialize(Node item, T& value)
    {
        using Type = typename ValueType<T>::Type;
        auto nodes = getChildren(item);

        value.clear();
        auto size = nodes.size();
        for (size_t i = 0; i < size; ++i) value.emplace_hint(value.end(), iget<Type>(nodes[i]));
    }
    template <typename T>
    EnableIfT<IsResizable<T>::value, void> deserialize(Node item, T& value)
    {
        size_t count = 0;
        auto nodes = getChildren(item);
        value.resize(nodes.size());
        for (auto& subvalue : value) deserialize(nodes[count++], subvalue);
    }
    template <typename T>
    void deserialize(Node item, PointerView<T>& value)
    {
        static_assert(std::is_same<decltype(value), PointerView<T>>::value, "Pointer view in not deserializable.");
    }
    template <typename T>
    EnableIfT<IsPointer<T>::value, void> deserialize(Node item, T& value)
    {
        auto valid = getAttribute(item, "valid");
        if (valid == "false") return value.reset();
        SERIO_ASSERT(valid == "true", "Invalid node attribute value detected.");

        auto nodes = getChildren(item);
        SERIO_ASSERT(nodes.size() == 1, "Expected xml nodes size to be one");
        auto* subvalue = new typename std::remove_const<typename T::element_type>::type;
        deserialize(nodes[0], *subvalue);
        value.reset(subvalue);
    }
    template <typename... Ts>
    void deserialize(Node item, std::chrono::duration<Ts...>& value)
    {
        using Type = std::chrono::duration<Ts...>;
        typename Type::rep temp;
        deserialize(item, temp);
        value = Type(temp);
    }
    template <typename... Ts>
    void deserialize(Node item, std::chrono::time_point<Ts...>& value)
    {
        using Type = std::chrono::time_point<Ts...>;
        typename Type::duration temp;
        deserialize(item, temp);
        value = Type(temp);
    }
    template <typename T>
    void deserialize(Node item, std::vector<bool, T>& value)
    {
        auto nodes = getChildren(item);
        value.resize(nodes.size());
        for (size_t i = 0; i < value.size(); ++i) value[i] = iget<bool>(nodes[i]);
    }
    template <typename T>
    EnableIfT<std::is_integral<T>::value || std::is_enum<T>::value, void> deserialize(Node item, T& value)
    {
        value = static_cast<T>(std::stoll(getString(item)));
    }
    template <typename T>
    EnableIfT<std::is_floating_point<T>::value, void> deserialize(Node item, T& value)
    {
        value = std::stod(getString(item));
    }
    inline void deserialize(Node item, uint64_t& value)  //
    {
        value = std::stoull(getString(item));
    }
    inline void deserialize(Node item, long double& value)  //
    {
        value = std::stold(getString(item));
    }
    inline void deserialize(Node item, bool& value)  //
    {
        auto str = getString(item);
        if (str == "true")
            value = true;
        else if (str == "false")
            value = false;
        else
            SERIO_ASSERT(false, "Invalid node value (should be true|false)");
    }
    template <typename... Ts>
    void deserialize(Node item, std::basic_string<char, Ts...>& value)
    {
        value = getString(item);
    }
    template <typename... Ts>
    void deserialize(Node item, std::basic_string<Ts...>& value)
    {
        fromutf8(getString(item), value);
    }
    template <size_t N>
    void deserialize(Node item, std::bitset<N>& value)
    {
        value = std::bitset<N>(getString(item));
    }
    template <typename T>
    void deserialize(Node item, std::weak_ptr<T>& value)
    {
        static_assert(std::is_same<decltype(value), std::weak_ptr<T>>::value, "Weak pointer in not deserializable");
    }
    template <typename... Ts>
    void deserialize(Node item, std::queue<Ts...>& value)
    {
        return deserialize(item, static_cast<Queue<Ts...>&>(value).c);
    }
    template <typename... Ts>
    void deserialize(Node item, std::stack<Ts...>& value)
    {
        return deserialize(item, static_cast<Stack<Ts...>&>(value).c);
    }
    template <typename... Ts>
    void deserialize(Node item, std::priority_queue<Ts...>& value)
    {
        return deserialize(item, static_cast<PQueue<Ts...>&>(value).c);
    }
    template <typename T1, typename T2>
    void deserialize(Node item, std::pair<T1, T2>& value)
    {
        auto nodes = getChildren(item);
        SERIO_ASSERT(nodes.size() == 2, "Expected xml nodes size to be two");
        deserialize(nodes[0], value.first);
        deserialize(nodes[1], value.second);
    }
    template <typename... Ts>
    void deserialize(Node item, std::tuple<Ts...>& value)
    {
        Derived deserializer(item->first_node());
        Tuple<sizeof...(Ts)>::deserialize(deserializer, value);
    }
    template <typename T>
    void deserialize(Node item, std::complex<T>& value)
    {
        auto nodes = getChildren(item);
        SERIO_ASSERT(nodes.size() == 2, "Expected xml nodes size to be two");
        value.real(iget<T>(nodes[0]));
        value.imag(iget<T>(nodes[1]));
    }
    template <typename T>
    void deserialize(Node item, std::atomic<T>& value)
    {
        value.store(iget<T>(item));
    }
    template <typename T>
    void deserialize(Node item, BinaryString<T> value)
    {
        auto input = item->value();
        auto insize = item->value_size();
        auto outsize = b64decSize(input, insize);
        std::string output(outsize, 0);
        base64Decode((const uint8_t*)input, (uint8_t*)output.data(), insize);
        fromutf8(output, value.value);
    }

#if SERIO_CPP_VERSION >= 201703L
    template <typename... Ts>
    void deserialize(Node item, std::basic_string_view<Ts...>& value)
    {
        static_assert(std::is_same<decltype(value), std::basic_string_view<Ts...>>::value,
                      "String view in not deserializable.");
    }
    template <typename T>
    EnableIfT<IsOptional<T>::value, void> deserialize(Node item, T& value)
    {
        auto valid = getAttribute(item, "valid");
        if (valid == "false") return value.reset();
        SERIO_ASSERT(valid == "true", "Invalid node attribute value detected.");

        auto nodes = getChildren(item);
        SERIO_ASSERT(nodes.size() == 1, "Expected xml nodes size to be one");
        value.emplace(iget<typename T::value_type>(nodes[0]));
    }
    template <typename... Ts>
    void deserialize(Node item, std::variant<Ts...>& value)
    {
        Derived deserializer(item->first_node());
        auto index = deserializer.template get<Size>();
        Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(deserializer, index, value);
    }
    inline void deserialize(Node item, std::monostate&)  //
    {
        SERIO_ASSERT(isNull(item), "Expected xml node to be null");
    }
#endif

#if SERIO_CPP_VERSION >= 202002L
    template <typename T, size_t S>
    void deserialize(Node item, std::span<T, S>& value)
    {
        static_assert(std::is_same<decltype(value), std::span<T, S>>::value, "Span in not deserializable.");
    }
#endif

#if SERIO_CPP_VERSION >= 202302L
    template <typename T, typename E>
    void deserialize(Node item, std::expected<T, E>& value)
    {
        auto valid = getAttribute(item, "valid");
        if (valid == "true")
        {
            auto nodes = getChildren(item);
            SERIO_ASSERT(nodes.size() == 1, "Expected xml nodes size to be one");
            value.emplace(iget<T>(nodes[0]));
        }
        else if (valid == "false")
        {
            auto nodes = getChildren(item);
            SERIO_ASSERT(nodes.size() == 1, "Expected xml nodes size to be one");
            value = std::unexpected(iget<E>(nodes[0]));
        }
        else
            SERIO_ASSERT(false, "Invalid node attribute value detected.");
    }
#endif

#if SERIO_ENABLE_FILESYSTEM
    void deserialize(Node item, std::filesystem::path& value)  //
    {
        std::string str;
        deserialize(item, binaryString(str));
        value = std::filesystem::path{std::move(str)};
    }
#endif

public:
    XmlDeserializer(Node node) : node(node) {}

    static XmlDeserializer parse(Document& document, StringView buffer)
    {
        try
        {
            document.parse<rapidxml::parse_fastest>((char*)buffer.data);
        }
        catch (const rapidxml::parse_error& e)
        {
            throw Serio::Exception(std::string("XML parse error: ") + e.what());
        }
        SERIO_ASSERT(document.first_node(), "XML document has no root element");
        return XmlDeserializer(document.first_node());
    }

    static XmlDeserializer parse(Document& document, std::istream& stream, std::string& buffer)
    {
        auto it = (std::istreambuf_iterator<char>(stream));
        buffer.assign(it, std::istreambuf_iterator<char>());
        return parse(document, buffer);
    }

    template <typename T>
    T get()
    {
        T value;
        *this >> value;
        return value;
    }

    Size getLength() { return this->get<Size>(); }

    template <typename T>
    Derived& operator>>(T& value)
    {
        SERIO_ASSERT(node, "Request more items from a finished xml deserializer");
        deserialize(node, value);
        node = node->next_sibling();
        return *this;
    }
    template <typename T>
    Derived& operator>>(BinaryString<T> value)
    {
        SERIO_ASSERT(node, "Request more items from a finished xml deserializer");
        deserialize(node, value);
        node = node->next_sibling();
        return *this;
    }
    template <typename T, Size N>
    Derived& operator>>(StaticArrayView<T, N> value)
    {
        SERIO_ASSERT(node, "Request more items from a finished xml deserializer");
        deserialize(node, value);
        node = node->next_sibling();
        return *this;
    }
    template <typename T>
    Derived& operator>>(NVP<T> value)
    {
        // TODO Can node be null when the parent has no child?
        auto item = node->parent()->first_node(value.name.data, value.name.size);
        SERIO_ASSERT(item, "The xml node doesn't contain the requested child");
        deserialize(item, value.value);
        return *this;
    }

    inline bool hasKey(const std::string& key)
    {
        auto item = node->parent()->first_node(key.data(), key.size());
        return bool(item);
    }

    template <typename Head, typename... Tail>
    Derived& process(Head&& head, Tail&&... tail)
    {
        *this >> std::forward<Head>(head);
        return process(std::forward<Tail>(tail)...);
    }
    Derived& process() { return *this; }
};
}  // namespace Impl
}  // namespace Serio
