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

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <serio/binary.h>

namespace Serio
{
namespace Impl
{
class JsonSerializer : public Base::Serializer, public Base::JSON
{
    using Derived = JsonSerializer;

    using Type = rapidjson::Type;
    using Value = rapidjson::GenericValue<rapidjson::UTF8<>>;
    using Allocator = rapidjson::GenericDocument<rapidjson::UTF8<>>::AllocatorType;

    enum class Stage
    {
        Empty,
        Single,
        Array,
        Object,
    };

    Allocator& allocator;
    Stage stage{Stage::Empty};

    template <typename T>
    EnableIfT<std::is_class<T>::value && !IsFixed<T>::value && !IsResizable<T>::value && !IsAppendable<T>::value &&
                  !IsPointer<T>::value && !IsOptional<T>::value,
              Value>
    serialize(const T& value)
    {
        Derived serializer(allocator);
        CustomClass<T>().serialize(value, serializer);
        return std::move(serializer.json);
    }
    template <typename T>
    EnableIfT<IsFixed<T>::value || IsResizable<T>::value || IsAppendable<T>::value, Value> serialize(const T& value)
    {
        Value output(Type::kArrayType);
        for (const auto& item : value) output.PushBack(serialize(item), allocator);
        return output;
    }
    template <typename T>
    EnableIfT<IsPointer<T>::value, Value> serialize(const T& value)
    {
        Value output(Type::kArrayType);
        output.PushBack(serialize(bool(value)), allocator);
        output.PushBack(value ? serialize(*value.get()) : Value(Type::kNullType), allocator);
        return output;
    }
    template <typename... Ts>
    Value serialize(const std::chrono::duration<Ts...>& value)
    {
        return serialize(value.count());
    }
    template <typename... Ts>
    Value serialize(const std::chrono::time_point<Ts...>& value)
    {
        return serialize(value.time_since_epoch());
    }

    template <typename T>
    Value serialize(const std::vector<bool, T>& value)
    {
        Value output(Type::kArrayType);
        for (const auto& item : value) output.PushBack(serialize(bool(item)), allocator);
        return output;
    }
    template <typename T>
    EnableIfT<std::is_integral<T>::value || std::is_enum<T>::value, Value> serialize(const T& value)
    {
        return Value(int64_t(value));
    }
    template <typename T>
    EnableIfT<std::is_floating_point<T>::value, Value> serialize(const T& value)
    {
        return Value(double(value));
    }
    inline Value serialize(const uint64_t& value)  //
    {
        return Value(value);
    }
    inline Value serialize(const bool& value)  //
    {
        return Value(value);
    }
    template <typename... Ts>
    Value serialize(const std::basic_string<char, Ts...>& value)
    {
        return Value(value.data(), value.size(), allocator);
    }
    template <typename... Ts>
    Value serialize(const std::basic_string<Ts...>& value)
    {
        return serialize(toutf8(value.data(), value.data() + value.size()));
    }
    template <size_t N>
    Value serialize(const std::bitset<N>& value)
    {
        return serialize(value.to_string());
    }
    template <typename... Ts>
    Value serialize(const std::queue<Ts...>& value)
    {
        return serialize(static_cast<const Queue<Ts...>&>(value).c);
    }
    template <typename... Ts>
    Value serialize(const std::stack<Ts...>& value)
    {
        return serialize(static_cast<const Stack<Ts...>&>(value).c);
    }
    template <typename... Ts>
    Value serialize(const std::priority_queue<Ts...>& value)
    {
        return serialize(static_cast<const PQueue<Ts...>&>(value).c);
    }
    template <typename T1, typename T2>
    Value serialize(const std::pair<T1, T2>& value)
    {
        Value output(Type::kArrayType);
        output.PushBack(serialize(value.first), allocator);
        output.PushBack(serialize(value.second), allocator);
        return output;
    }
    template <typename... Ts>
    Value serialize(const std::tuple<Ts...>& value)
    {
        Derived serializer(allocator);
        serializer.startArray();
        Tuple<sizeof...(Ts)>::serialize(serializer, value);
        return std::move(serializer.json);
    }
    template <typename T>
    Value serialize(const std::complex<T>& value)
    {
        Value output(Type::kArrayType);
        output.PushBack(serialize(value.real()), allocator);
        output.PushBack(serialize(value.imag()), allocator);
        return output;
    }
    template <typename... Ts>
    Value serialize(const std::atomic<Ts...>& value)
    {
        return serialize(value.load());
    }
    template <typename T>
    Value serialize(const BinaryString<T>& value)
    {
        return serializeBinary(value.value.data(), value.value.size());
    }
    template <typename... Ts>
    Value serializeBinary(const char* data, size_t insize)
    {
        if (insize == 0) return Value(Type::kStringType);
        auto outsize = b64encSize(insize);
        auto output = (char*)allocator.Malloc(outsize);
        base64Encode((const uint8_t*)data, output, insize);
        return Value(output, outsize);
    }
    template <typename T>
    Value serializeBinary(const T* data, size_t size)
    {
        auto str = toutf8(data, data + size);
        return serializeBinary(str.data(), str.size());
    }

#if SERIO_CPP_VERSION >= 201703L
    template <typename... Ts>
    Value serialize(const std::basic_string_view<Ts...>& value)
    {
        return serialize(toutf8(value.data(), value.data() + value.size()));
    }
    template <typename T>
    EnableIfT<IsOptional<T>::value, Value> serialize(const T& value)
    {
        Value output(Type::kArrayType);
        output.PushBack(serialize(bool(value)), allocator);
        output.PushBack(value ? serialize(value.value()) : Value(Type::kNullType), allocator);
        return output;
    }
    template <typename... Ts>
    Value serialize(const std::variant<Ts...>& value)
    {
        Value output(Type::kArrayType);
        output.PushBack(serialize(value.index()), allocator);
        std::visit([&](auto&& arg) { output.PushBack(serialize(arg), allocator); }, value);
        return output;
    }
    inline Value serialize(const std::monostate&) { return Value(Type::kNullType); }
#endif

#if SERIO_CPP_VERSION >= 202302L
    template <typename T, typename E>
    inline Value serialize(const std::expected<T, E>& value)
    {
        Value output(Type::kArrayType);
        output.PushBack(serialize(value.has_value()), allocator);
        output.PushBack(value ? serialize(value.value()) : serialize(value.error()), allocator);
        return output;
    }
#endif

#if SERIO_ENABLE_FILESYSTEM
    Value serialize(const std::filesystem::path& value)  //
    {
        return serialize(binaryString(value.string()));
    }
#endif

public:
    using Document = rapidjson::GenericDocument<rapidjson::UTF8<>>;

    Value json{Type::kNullType};

    inline JsonSerializer(Allocator& allocator) : allocator(allocator) {}

    template <typename T>
    Derived& operator<<(const T& value)
    {
        if (stage == Stage::Array)
            json.PushBack(serialize(value), allocator);
        else if (stage == Stage::Empty)
        {
            json = serialize(value);
            stage = Stage::Single;
        }
        else if (stage == Stage::Single)
        {
            Value output(Type::kArrayType);
            output.PushBack(json, allocator);
            output.PushBack(serialize(value), allocator);
            json = output;
            stage = Stage::Array;
        }
        SERIO_ASSERT(stage != Stage::Object,
                     "Only NVP can be serialized when the serializer is in object writing mode");
        return *this;
    }

    template <typename T>
    Derived& operator<<(const NVP<T>& nvp)
    {
        if (stage == Stage::Empty)
        {
            json.SetObject();
            stage = Stage::Object;
        }
        SERIO_ASSERT(stage == Stage::Object, "The serializer must be in object writing mode to serialize NVP");
        Value key(nvp.name.data, nvp.name.size, allocator);
        json.AddMember(key, serialize(nvp.value), allocator);
        return *this;
    }

    inline void startArray()
    {
        if (stage == Stage::Empty)
            json.SetArray();
        else if (stage == Stage::Single)
        {
            Value output(Type::kArrayType);
            output.PushBack(json, allocator);
            json = output;
        }
        SERIO_ASSERT(stage != Stage::Object, "The serializer can't switch from object to array writing mode");
        stage = Stage::Array;
    }

    template <typename Head, typename... Tail>
    void _process(Head&& head, Tail&&... tail)
    {
        *this << std::forward<Head>(head);
        return _process(std::forward<Tail>(tail)...);
    }
    inline void _process() {}

    template <typename... Ts>
    Derived& process(Ts&&... ts)
    {
        // Do not pre-call startArray(): NVP items need Object mode, which the
        // NVP operator<< sets up on first call.  Non-NVP items self-promote from
        // Single to Array mode via the regular operator<<, so the result is the same.
        _process(std::forward<Ts>(ts)...);
        return *this;
    }

    std::string finalize(bool compact)
    {
        using namespace rapidjson;
        Derived::Document document;
        GenericStringBuffer<UTF8<>> buffer;

        if (compact)
        {
            Writer<decltype(buffer), UTF8<>, UTF8<>> writer(buffer);
            json.Accept(writer);
        }
        else
        {
            PrettyWriter<decltype(buffer), UTF8<>, UTF8<>> writer(buffer);
            json.Accept(writer);
        }
        return buffer.GetString();
    }

    void finalize(std::ostream& stream, bool compact)
    {
        using namespace rapidjson;
        BasicOStreamWrapper<std::ostream> wrapper(stream);

        if (compact)
        {
            Writer<decltype(wrapper), UTF8<>, UTF8<>> writer(wrapper);
            json.Accept(writer);
        }
        else
        {
            PrettyWriter<decltype(wrapper), UTF8<>, UTF8<>> writer(wrapper);
            json.Accept(writer);
        }
    }
};

class JsonDeserializer : public Base::Deserializer, public Base::JSON
{
    using Derived = JsonDeserializer;

    using Type = rapidjson::Type;
    using Value = rapidjson::GenericValue<rapidjson::UTF8<>>;

    enum class Stage
    {
        Value,
        Array,
        Object,
    };

    Value& json;
    Size index = 0;
    Stage stage{Stage::Value};

    template <typename T>
    T iget(Value& item)
    {
        T value;
        deserialize(item, value);
        return value;
    }

    template <typename T>
    EnableIfT<std::is_class<T>::value && !IsFixed<T>::value && !IsResizable<T>::value && !IsAppendable<T>::value &&
                  !IsPointer<T>::value && !IsOptional<T>::value,
              void>
    deserialize(Value& item, T& value)
    {
        Derived deserializer(item);
        CustomClass<T>().deserialize(value, deserializer);
    }
    template <typename T>
    EnableIfT<IsFixed<T>::value, void> deserialize(Value& item, T& value)
    {
        auto size = item.Size();
        SERIO_ASSERT(item.IsArray(), "Expected json value to be array");
        SERIO_ASSERT(size == IsFixed<T>::size(), "Invalid array size encountered");
        for (size_t i = 0; i < size; ++i) deserialize(item[i], value[i]);
    }
    template <typename T>
    EnableIfT<IsAppendable<T>::value, void> deserialize(Value& item, T& value)
    {
        SERIO_ASSERT(item.IsArray(), "Expected json value to be array");
        using Type = typename ValueType<T>::Type;

        value.clear();
        auto size = item.Size();
        for (size_t i = 0; i < size; ++i) value.emplace_hint(value.end(), iget<Type>(item[i]));
    }
    template <typename T>
    EnableIfT<IsResizable<T>::value, void> deserialize(Value& item, T& value)
    {
        SERIO_ASSERT(item.IsArray(), "Expected json value to be array");
        size_t count = 0;
        value.resize(item.Size());
        for (auto& subvalue : value) deserialize(item[count++], subvalue);
    }
    template <typename T>
    void deserialize(Value& item, PointerView<T>& value)
    {
        static_assert(std::is_same<decltype(value), PointerView<T>>::value, "Pointer view in not deserializable.");
    }
    template <typename T>
    EnableIfT<IsPointer<T>::value, void> deserialize(Value& item, T& value)
    {
        Derived deserializer(item);
        deserializer.startTuple(2);
        bool valid = false;
        deserializer >> valid;
        if (!valid) return value.reset();

        auto* subvalue = new typename std::remove_const<typename T::element_type>::type;
        deserializer >> *subvalue;
        value.reset(subvalue);
    }
    template <typename... Ts>
    void deserialize(Value& item, std::chrono::duration<Ts...>& value)
    {
        using Type = std::chrono::duration<Ts...>;
        typename Type::rep temp;
        deserialize(item, temp);
        value = Type(temp);
    }
    template <typename... Ts>
    void deserialize(Value& item, std::chrono::time_point<Ts...>& value)
    {
        using Type = std::chrono::time_point<Ts...>;
        typename Type::duration temp;
        deserialize(item, temp);
        value = Type(temp);
    }

    template <typename T>
    void deserialize(Value& item, std::vector<bool, T>& value)
    {
        SERIO_ASSERT(item.IsArray(), "Expected json value to be array");
        value.resize(item.Size());
        for (size_t i = 0; i < value.size(); ++i) value[i] = iget<bool>(item[i]);
    }
    template <typename T>
    EnableIfT<std::is_integral<T>::value || std::is_enum<T>::value, void> deserialize(Value& item, T& value)
    {
        SERIO_ASSERT(item.IsInt() || item.IsInt64() || item.IsUint() || item.IsUint64(),
                     "Expected json value to be integer");
        value = static_cast<T>(item.GetInt64());
    }
    template <typename T>
    EnableIfT<std::is_floating_point<T>::value, void> deserialize(Value& item, T& value)
    {
        SERIO_ASSERT(item.IsNumber(), "Expected json value to be number");
        value = item.GetDouble();
    }
    inline void deserialize(Value& item, uint64_t& value)
    {
        SERIO_ASSERT(item.IsUint() || item.IsUint64(), "Expected json value to be unsigned integer");
        value = item.GetUint64();
    }
    inline void deserialize(Value& item, bool& value)
    {
        SERIO_ASSERT(item.IsBool(), "Expected json value to be boolean");
        value = item.GetBool();
    }
    template <typename... Ts>
    void deserialize(Value& item, std::basic_string<char, Ts...>& value)
    {
        SERIO_ASSERT(item.IsString(), "Expected json value to be string");
        value.assign(item.GetString(), item.GetStringLength());
    }
    template <typename... Ts>
    void deserialize(Value& item, std::basic_string<Ts...>& value)
    {
        std::string data;
        deserialize(item, data);
        fromutf8(data, value);
    }
    template <size_t N>
    void deserialize(Value& item, std::bitset<N>& value)
    {
        SERIO_ASSERT(item.IsString(), "Expected json value to be string");
        value = std::bitset<N>(item.GetString());
    }
    template <typename T>
    void deserialize(Value& item, std::weak_ptr<T>& value)
    {
        static_assert(std::is_same<decltype(value), std::weak_ptr<T>>::value, "Weak pointer in not deserializable.");
    }
    template <typename... Ts>
    void deserialize(Value& item, std::queue<Ts...>& value)
    {
        return deserialize(item, static_cast<Queue<Ts...>&>(value).c);
    }
    template <typename... Ts>
    void deserialize(Value& item, std::stack<Ts...>& value)
    {
        return deserialize(item, static_cast<Stack<Ts...>&>(value).c);
    }
    template <typename... Ts>
    void deserialize(Value& item, std::priority_queue<Ts...>& value)
    {
        return deserialize(item, static_cast<PQueue<Ts...>&>(value).c);
    }
    template <typename T1, typename T2>
    void deserialize(Value& item, std::pair<T1, T2>& value)
    {
        Derived deserializer(item);
        deserializer.startTuple(2);
        deserializer >> value.first >> value.second;
    }
    template <typename... Ts>
    void deserialize(Value& item, std::tuple<Ts...>& value)
    {
        Derived deserializer(item);
        deserializer.startTuple(sizeof...(Ts));
        Tuple<sizeof...(Ts)>::deserialize(deserializer, value);
    }
    template <typename T>
    void deserialize(Value& item, std::complex<T>& value)
    {
        Derived deserializer(item);
        deserializer.startTuple(2);
        value.real(deserializer.get<T>());
        value.imag(deserializer.get<T>());
    }
    template <typename T>
    void deserialize(Value& item, std::atomic<T>& value)
    {
        value.store(iget<T>(item));
    }
    template <typename T>
    void deserialize(Value& item, BinaryString<T> value)
    {
        auto input = item.GetString();
        auto insize = item.GetStringLength();
        auto outsize = b64decSize(input, insize);
        std::string output(outsize, 0);
        base64Decode((const uint8_t*)input, (uint8_t*)output.data(), insize);
        fromutf8(output, value.value);
    }

#if SERIO_CPP_VERSION >= 201703L
    template <typename... Ts>
    void deserialize(Value& item, std::basic_string_view<Ts...>& value)
    {
        static_assert(std::is_same<decltype(value), std::basic_string_view<Ts...>>::value,
                      "String view in not deserializable.");
    }
    template <typename T>
    EnableIfT<IsOptional<T>::value, void> deserialize(Value& item, T& value)
    {
        Derived deserializer(item);
        deserializer.startTuple(2);
        bool valid = false;
        deserializer >> valid;
        if (!valid) return value.reset();

        typename T::value_type subvalue;
        deserializer >> subvalue;
        value.emplace(std::move(subvalue));
    }
    template <typename... Ts>
    void deserialize(Value& item, std::variant<Ts...>& value)
    {
        Derived deserializer(item);
        deserializer.startTuple(2);
        auto index = deserializer.template get<Size>();
        Variant<std::tuple<Ts...>, sizeof...(Ts) - 1>::deserialize(deserializer, index, value);
    }
    inline void deserialize(Value& item, std::monostate&)  //
    {
        SERIO_ASSERT(item.IsNull(), "Expected json value to be null");
    }
#endif

#if SERIO_CPP_VERSION >= 202002L
    template <typename T, size_t S>
    void deserialize(Value& item, std::span<T, S>& value)
    {
        static_assert(std::is_same<decltype(value), std::span<T, S>>::value, "Span in not deserializable.");
    }
#endif

#if SERIO_CPP_VERSION >= 202302L
    template <typename T, typename E>
    inline void deserialize(Value& item, std::expected<T, E>& value)
    {
        Derived deserializer(item);
        deserializer.startTuple(2);
        bool valid = false;
        deserializer >> valid;

        if (valid)
        {
            T subvalue;
            deserializer >> subvalue;
            value.emplace(std::move(subvalue));
        }
        else
        {
            E subvalue;
            deserializer >> subvalue;
            value = std::unexpected(std::move(subvalue));
        }
    }
#endif

#if SERIO_ENABLE_FILESYSTEM
    void deserialize(Value& item, std::filesystem::path& value)  //
    {
        std::string str;
        deserialize(item, binaryString(str));
        value = std::filesystem::path{std::move(str)};
    }
#endif

public:
    using Document = rapidjson::GenericDocument<rapidjson::UTF8<>>;

    JsonDeserializer(Value& value) : json(value) {}

    inline static JsonDeserializer parse(Document& document, StringView buffer)
    {
        document.Parse(buffer.data, buffer.size);
        SERIO_ASSERT(!document.HasParseError(), "Failed to parse json document");
        return JsonDeserializer(document);
    }

    inline static JsonDeserializer parse(Document& document, std::istream& stream)
    {
        using namespace rapidjson;
        BasicIStreamWrapper<std::istream> wrapper(stream);
        document.ParseStream(wrapper);
        SERIO_ASSERT(!document.HasParseError(), "Failed to parse json document");
        return JsonDeserializer(document);
    }

    template <typename T>
    T get()
    {
        T value;
        *this >> value;
        return value;
    }

    Size getLength() { return this->get<Size>(); }

    inline void startTuple(Size count)  //
    {
        SERIO_ASSERT(startArray() == count, "Expected json array size to match tuple size");
    }

    inline Size startArray()
    {
        SERIO_ASSERT(json.IsArray(), "Expected json value to be array");
        stage = Stage::Array;
        return json.Size();
    }

    inline void startObject()
    {
        SERIO_ASSERT(json.IsObject(), "Expected json value to be object");
        stage = Stage::Object;
    }

    inline bool hasKey(const std::string& key) { return json.HasMember(key.c_str()); }

    template <typename T>
    Derived& operator>>(T& value)
    {
        if (stage == Stage::Value)
            deserialize(json, value);
        else if (stage == Stage::Array)
        {
            SERIO_ASSERT(index < json.Size(), "Request more items from a finished json deserializer");
            deserialize(json[index++], value);
        }
        SERIO_ASSERT(stage != Stage::Object, "Object reading mode of deserializer only supports NVP deserialization");

        return *this;
    }

    template <typename T>
    Derived& operator>>(BinaryString<T> value)
    {
        if (stage == Stage::Value)
            deserialize(json, value);
        else if (stage == Stage::Array)
        {
            SERIO_ASSERT(index < json.Size(), "Request more items from a finished json deserializer");
            deserialize(json[index++], value);
        }
        SERIO_ASSERT(stage != Stage::Object, "Object reading mode of deserializer only supports NVP deserialization");

        return *this;
    }

    template <typename T, Size N>
    Derived& operator>>(StaticArrayView<T, N> value)
    {
        if (stage == Stage::Value)
            deserialize(json, value);
        else if (stage == Stage::Array)
        {
            SERIO_ASSERT(index < json.Size(), "Request more items from a finished json deserializer");
            deserialize(json[index++], value);
        }
        SERIO_ASSERT(stage != Stage::Object, "Object reading mode of deserializer only supports NVP deserialization");
        return *this;
    }

    template <typename T>
    Derived& operator>>(NVP<T> value)
    {
        SERIO_ASSERT(stage == Stage::Object, "The deserializer must be in object reading mode to get NVP from it");
        SERIO_ASSERT(json.HasMember(value.name.data), "The object doesn't contain the requested key");
        deserialize(json[value.name.data], value.value);
        return *this;
    }

    template <typename Head, typename... Tail>
    void _process(Head&& head, Tail&&... tail)
    {
        *this >> std::forward<Head>(head);
        return _process(std::forward<Tail>(tail)...);
    }
    inline void _process() {}

    template <typename... Ts>
    Derived& process(Ts&&... ts)
    {
        // If the JSON value is an object, switch to Object mode so NVP items can
        // be deserialized by key.  Otherwise treat it as an array (tuple) as before.
        if (sizeof...(Ts) > 1)
        {
            if (json.IsObject())
                startObject();
            else
                startTuple(sizeof...(Ts));
        }
        _process(std::forward<Ts>(ts)...);
        return *this;
    }
};
}  // namespace Impl
}  // namespace Serio
