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

#ifdef _MSVC_LANG
#define SERIO_CPP_VERSION _MSVC_LANG
#else
#define SERIO_CPP_VERSION __cplusplus
#endif

#include <serio/config.h>

#include <cstdint>
#include <cstring>
#include <exception>
#include <string>
#include <type_traits>
#include <utility>
//
#if SERIO_CPP_VERSION >= 201703L
#include <string_view>
#endif

#if __has_include(<filesystem>)
#include <filesystem>
#if defined(__cpp_lib_filesystem)
/// Defined when `<filesystem>` is available and `std::filesystem` is fully supported by the
/// compiler. When defined, `std::filesystem::path` becomes a serializable type.
#define SERIO_ENABLE_FILESYSTEM 1
#endif
#endif

#ifdef _WIN32
/// Expands to `__declspec(dllexport)` on Windows so that symbols in a shared-library build are
/// exported. On all other platforms it expands to nothing.
#define SERIO_API __declspec(dllexport)
#else
#define SERIO_API
#endif

/// Injects friend declarations for every internal serializer and deserializer class into the
/// enclosing type, granting them access to private and protected members. `SERIO_REGISTER` already
/// expands this macro internally, so you only need to place `SERIO_FRIEND` directly in a class
/// body when you write `_serialize` and `_deserialize` by hand instead of using `SERIO_REGISTER`.
#define SERIO_FRIEND                            \
    template <typename _T>                      \
    friend class Serio::Impl::SerializerOps;    \
    template <typename _T>                      \
    friend class Serio::Impl::DeserializerOps;  \
                                                \
    friend class Serio::Impl::JsonSerializer;   \
    friend class Serio::Impl::JsonDeserializer; \
    friend class Serio::Impl::XmlSerializer;    \
    friend class Serio::Impl::XmlDeserializer;

/// Registers a class for serialization by generating `_serialize` and `_deserialize` template
/// methods and the necessary friend declarations. Place this macro in the body of the class you
/// want to serialize, passing each member variable (or base-class sub-object) that should
/// participate in serialization as arguments.
///
/// Example usage:
/// @code
/// struct Point {
///     float x, y;
///     SERIO_REGISTER(x, y)
/// };
/// @endcode
///
/// The generated `_serialize` and `_deserialize` methods forward all listed members to the
/// serializer/deserializer via its `process()` variadic call, so members are processed in the
/// order they are listed. The same order must be maintained across versions for binary
/// compatibility.
///
/// Internally this macro also expands `SERIO_FRIEND` so private members are always accessible.
#define SERIO_REGISTER(...)              \
    SERIO_FRIEND                         \
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

/// Expands to the most descriptive function-signature string available for the current compiler:
/// `__PRETTY_FUNCTION__` on GCC/Clang, `__FUNCSIG__` on MSVC, and `__func__` otherwise.
/// Used inside `SERIO_ASSERT` to produce informative error messages.
#if defined(__GNUC__) || defined(__clang__)
#define SERIO_FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define SERIO_FUNCTION_NAME __FUNCSIG__
#else
#define SERIO_FUNCTION_NAME __func__
#endif

/// Defined on Unix-like platforms (Linux, macOS, BSD). When present, the library uses
/// `mmap`-based file reading in `Serio::load()` instead of reading the entire file into a heap
/// buffer, which avoids a copy and can be significantly faster for large files.
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__) || defined(BSD)
#define SERIO_UNIX
#endif

/// Evaluates `expr` and, if it is false, throws `Serio::Exception` with a message that includes
/// the source file name, line number, enclosing function signature, and the user-supplied `msg`
/// string. This macro is used pervasively inside the library to validate invariants during
/// serialization and deserialization (e.g., checking that a buffer has enough remaining bytes,
/// that JSON node types match expectations, and that version numbers are compatible).
///
/// @param expr  A boolean-convertible expression. If it evaluates to false an exception is thrown.
/// @param msg   A `const char*` or `std::string`-constructible literal describing the failure.
#define SERIO_ASSERT(expr, msg)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!static_cast<bool>(expr))                                                                                  \
        {                                                                                                              \
            throw Serio::Exception(std::string("Assertion at ") + __FILE__ + ":" + std::to_string(__LINE__) + " -> " + \
                                   SERIO_FUNCTION_NAME + ":\n\t" + msg);                                               \
        }                                                                                                              \
    } while (false)

namespace Serio
{
/// The unsigned integer type used to represent container sizes, byte counts, and offsets
/// throughout the library. Fixed at 64 bits so that binary files produced on 32-bit platforms
/// can be read on 64-bit ones and vice versa.
using Size = std::uint64_t;

/// The exception type thrown by `SERIO_ASSERT`. Every runtime error the library detects —
/// malformed binary data, version mismatches, type mismatches in JSON/XML, failed file I/O,
/// CRC check failures, etc. — is reported through this type. Catch `Serio::Exception` (or its
/// base `std::exception`) to handle all library errors uniformly.
struct Exception : std::exception
{
    /// Human-readable description of the error. When the exception is thrown via `SERIO_ASSERT`
    /// this string includes the source file, line number, and function signature; when constructed
    /// directly it contains whatever string was passed to the constructor.
    std::string message;

    /// Constructs an exception with the given message string.
    inline explicit Exception(std::string message) : message(std::move(message)) {}

    /// Returns `message.c_str()`, implementing the `std::exception` interface.
    inline const char* what() const noexcept override { return message.c_str(); }
};

/// A non-owning view over a contiguous block of elements of type `T`, analogous to a pre-C++20
/// `std::span`. It carries a raw pointer and an element count but does not manage memory lifetime.
/// Use it to serialize a typed array that already lives in an existing allocation — for example, a
/// raw `float*` buffer — without copying the data into a standard container first.
///
/// Serializing a `PointerView` writes the element count followed by the raw bytes of all elements
/// (using the fast bulk-copy path on little-endian systems when `T` is arithmetic). The iterator
/// interface (`data()`, `begin()`, `end()`) is const-only; mutation goes through `operator[]` or
/// directly via the `ptr` member.
///
/// Deserializing into a `PointerView` is intentionally unsupported: the view has no mechanism to
/// resize or reallocate memory, so a compile-time `static_assert` is triggered if you attempt it.
/// Use `StaticArrayView` instead when the buffer already exists and its size is known at compile
/// time, or a `std::vector` when you need the deserialization to allocate memory.
///
/// @tparam T  Element type. Must be arithmetic for the fast bulk-copy path to be used.
template <typename T>
struct PointerView
{
    /// Pointer to the first element of the viewed region. Must be non-null when `length > 0`.
    T* ptr{nullptr};

    /// Number of elements (not bytes) in the viewed region.
    Size length{0};

    /// The element type alias required by the library's container traits (e.g., `IsContinuous`)
    /// to select the appropriate serialization strategy for this type.
    using value_type = T;

    /// Constructs an empty view with a null pointer and zero length.
    PointerView() = default;

    /// Constructs a view over `length` elements starting at `ptr`.
    PointerView(T* ptr, Size length) : ptr(ptr), length(length) {}

    /// Returns the number of elements in the viewed region. This method satisfies the
    /// `IsContinuous` container trait requirement used by the serializer to select the
    /// bulk-copy path for arithmetic element types.
    Size size() const { return length; }

    /// Returns a const pointer to the first element.
    const T* data() const { return ptr; }

    /// Returns a const pointer to the first element (iterator interface).
    const T* begin() const { return ptr; }

    /// Returns a const pointer one past the last element, or the null pointer if the view is
    /// empty.
    const T* end() const { return ptr ? ptr + length : ptr; }

    /// Returns a mutable reference to the element at index `i`. No bounds checking is performed.
    T& operator[](Size i) { return ptr[i]; }

    /// Returns a const reference to the element at index `i`. No bounds checking is performed.
    const T& operator[](Size i) const { return ptr[i]; }
};

/// A non-owning view over a fixed-size contiguous array of `N` elements of type `T`, where `N`
/// is a compile-time constant. Use it to serialize or deserialize a typed C array without wrapping
/// it in a `std::array` — the backing memory is provided by the caller and must remain valid for
/// the duration of the operation.
///
/// Because the size is baked into the type, no length prefix is written during serialization
/// (matching the behavior of `std::array`). During deserialization the library writes directly
/// into the pointed-to memory: for arithmetic `T` it uses a single bulk `memcpy` on little-endian
/// systems; for other types it deserializes element by element. This contrasts with `PointerView`,
/// which is serialization-only and triggers a compile-time error if you attempt to deserialize
/// into it.
///
/// @tparam T  Element type.
/// @tparam N  Number of elements, fixed at compile time.
template <typename T, Size N>
struct StaticArrayView
{
    /// Pointer to the first element of the viewed array. Must point to at least `N` valid,
    /// writable elements before passing this view to a deserializer.
    T* ptr{nullptr};

    /// The element type alias required by the library's container traits to select the appropriate
    /// serialization strategy for this type.
    using value_type = T;

    /// Constructs an empty (null) view.
    StaticArrayView() = default;

    /// Constructs a view over the `N`-element array starting at `ptr`.
    StaticArrayView(T* ptr) : ptr(ptr) {}

    /// Returns the fixed number of elements `N`.
    Size size() const { return N; }

    /// Returns a mutable pointer to the first element.
    T* data() { return ptr; }

    /// Returns a const pointer to the first element.
    const T* data() const { return ptr; }

    /// Returns a mutable pointer to the first element (iterator interface).
    T* begin() { return ptr; }

    /// Returns a const pointer to the first element (iterator interface).
    const T* begin() const { return ptr; }

    /// Returns a mutable pointer one past the last element, or null if the view is empty.
    T* end() { return ptr ? ptr + N : ptr; }

    /// Returns a const pointer one past the last element, or null if the view is empty.
    const T* end() const { return ptr ? ptr + N : ptr; }

    /// Returns a mutable reference to element `i`. No bounds checking is performed.
    T& operator[](Size i) { return ptr[i]; }

    /// Returns a const reference to element `i`. No bounds checking is performed.
    const T& operator[](Size i) const { return ptr[i]; }
};

/// A lightweight, non-owning view over a sequence of `char` bytes. Unlike `std::string_view`
/// (which requires C++17), this type is available in all supported language versions and is used
/// internally throughout the library to pass raw byte buffers — headers, serialized payloads,
/// memory-mapped file contents — without copying.
///
/// The `view()` member function returns a sub-slice, which is how the library clips the
/// payload region out of a buffer after consuming the header bytes.
struct StringView
{
    /// Pointer to the first character. May be null for an empty or uninitialized view.
    const char* data{nullptr};

    /// Number of bytes in the view.
    Size size{0};

    /// Constructs a view from a null-terminated C string (length computed via strlen).
    inline StringView(const char* data) : data(data), size(std::strlen(data)) {}

    /// Constructs a view from a raw pointer and byte count.
    inline StringView(const char* data, Size size) : data(data), size(size) {}

    /// Constructs a view that aliases the contents of a `std::string`. The string must remain
    /// alive for the lifetime of this view.
    inline StringView(const std::string& data) : data(data.data()), size(data.size()) {}

#if SERIO_CPP_VERSION >= 201703L
    /// Constructs a view from a `std::string_view` (C++17 and later only).
    inline StringView(std::string_view data) : data(data.data()), size(data.size()) {}
#endif

    /// Returns the character at position `i`. No bounds checking is performed.
    inline char operator[](Size i) { return data[i]; }

    /// Returns a sub-view covering the half-open byte range `[start, end)`. If `end` is omitted
    /// (or equal to `Size(-1)`), the sub-view extends to the end of this view. Used internally
    /// to produce the payload slice after stripping the binary header.
    inline StringView view(Size start, Size end = Size(-1))
    {
        if (end == Size(-1)) end = size;
        return StringView(data + start, end - start);
    }
};

/// The primary extension point for registering types that are not owned by you — types from
/// third-party libraries or system headers — for serialization without modifying them. When the
/// serializer encounters a class type `T` that is not handled by any built-in rule, it
/// instantiates `CustomClass<T>` and calls either `serialize` or `deserialize` on it.
///
/// The default implementation delegates to `value._serialize(C)` and `value._deserialize(C)`,
/// which are the methods injected by the `SERIO_REGISTER` macro. If your own class uses
/// `SERIO_REGISTER`, you do not need to specialize this template.
///
/// To make a type you don't own serializable, specialize `CustomClass<YourType>` and provide
/// `serialize` and `deserialize` implementations:
/// @code
/// namespace Serio {
///     template <>
///     struct CustomClass<ThirdPartyVec3> {
///         template <typename S> void serialize(const ThirdPartyVec3& v, S& C) {
///             C.process(v.x, v.y, v.z);
///         }
///         template <typename D> void deserialize(ThirdPartyVec3& v, D& C) {
///             C.process(v.x, v.y, v.z);
///         }
///     };
/// }
/// @endcode
///
/// The second template parameter `Enable` is available for SFINAE-based partial specializations
/// when you want to match a whole family of types at once.
///
/// @tparam T       The user-defined class type to serialize.
/// @tparam Enable  Unused by the default; reserved for SFINAE-based partial specializations.
template <typename T, class Enable = void>
struct CustomClass
{
    /// Calls `value._serialize(C)`, which is generated by `SERIO_REGISTER` inside the class body.
    template <typename Serializer>
    void serialize(const T& value, Serializer& C)
    {
        value._serialize(C);
    }

    /// Calls `value._deserialize(C)`, which is generated by `SERIO_REGISTER` inside the class body.
    template <typename Deserializer>
    void deserialize(T& value, Deserializer& C)
    {
        value._deserialize(C);
    }
};

/// A tag wrapper that instructs the serializer to treat the contained string value as opaque
/// binary data rather than human-readable text. In the binary backend this has no visible effect
/// because all strings are already stored as raw bytes. In the JSON and XML backends, however,
/// a bare string is assumed to be valid UTF-8 text, whereas a `BinaryString`-wrapped value is
/// base64-encoded so arbitrary byte sequences can be round-tripped safely.
///
/// Do not construct this directly; use the `binaryString()` helper instead.
///
/// @tparam T  Either `const SomeString&` or `SomeString&`, bound by the `binaryString()` factory.
template <typename T>
struct BinaryString
{
    /// The wrapped string value. `T` is a reference type so no copy is made.
    T value;
};

/// Creates a `BinaryString` wrapper around a const string value, instructing the serializer to
/// base64-encode it in JSON/XML output instead of treating it as UTF-8 text. Use this when the
/// string contains arbitrary binary data (e.g., a serialized blob, a cryptographic key, image
/// bytes).
///
/// @tparam T  Any string-like type with `.data()` and `.size()` members.
template <typename T>
BinaryString<const T&> binaryString(const T& value)
{
    return BinaryString<const T&>{value};
}

/// Creates a `BinaryString` wrapper around a mutable string reference, used on the
/// deserialization side to decode a base64 string from JSON/XML back into raw bytes and write
/// the result into `value`.
///
/// @tparam T  Any string-like type with `.data()` and `.size()` members.
template <typename T>
BinaryString<T&> binaryString(T& value)
{
    return BinaryString<T&>{value};
}

/// A Name-Value Pair that associates a string key with a value reference. When the JSON or
/// XML serializer encounters an `NVP`, it writes the value as a named field in a JSON object or
/// as an XML element with a tag equal to the name, rather than as a positional array element.
/// This makes the output human-readable and allows fields to be identified by name rather than
/// position.
///
/// In the binary backend only the value is serialized; the name is not written to the stream.
///
/// Create `NVP` instances using the `nvp()` helper rather than constructing them directly.
///
/// @tparam T  The (possibly reference-qualified) type of the stored value.
template <typename T>
struct NVP
{
    /// The string name used as the JSON object key or XML element tag.
    StringView name;

    /// The value to serialize or deserialize. T is a reference type when created via nvp().
    T value;
};

/// Creates an `NVP` binding `name` to a mutable `value`.
template <typename T>
NVP<T&> nvp(StringView name, T& value)
{
    return NVP<T&>{name, value};
}

/// Creates an `NVP` binding `name` to a const `value`.
template <typename T>
NVP<const T&> nvp(StringView name, const T& value)
{
    return NVP<const T&>{name, value};
}

/// Empty tag types that serve as base classes for every concrete serializer and deserializer in
/// the library. The `IfBinary`, `IfJSON`, `IfXML`, and their negations use
/// `std::is_base_of` checks against these tags to enable or disable `CustomClass`
/// specializations for specific backends via SFINAE.
namespace Base
{
/// Tag base inherited by all concrete serializer classes. Used by `IfBinary`, `IfJSON`, and
/// `IfXML` to detect the active backend in `CustomClass` SFINAE specializations.
struct Serializer
{
};
/// Tag base inherited by all concrete deserializer classes. Used by `IfBinary`, `IfJSON`, and
/// `IfXML` to detect the active backend in `CustomClass` SFINAE specializations.
struct Deserializer
{
};
/// Tag base that identifies a serializer or deserializer as operating in the binary format.
struct Binary
{
};
/// Tag base that identifies a serializer or deserializer as operating in the JSON format.
struct JSON
{
};
/// Tag base that identifies a serializer or deserializer as operating in the XML format.
struct XML
{
};
}  // namespace Base

namespace Impl
{
/// Internal alias for `std::enable_if<B, T>::type`. Used throughout the library for SFINAE
/// overload selection without depending on C++14's `std::enable_if_t`.
template <bool B, class T = void>
using EnableIfT = typename std::enable_if<B, T>::type;
}  // namespace Impl

/// Resolves to `void` when `T` is a binary-format serializer or deserializer (i.e., derives from
/// `Base::Binary`). Use this in the `Enable` parameter of a `CustomClass` partial specialization
/// to restrict the specialization to the binary backend only.
template <typename T>
using IfBinary = Impl::EnableIfT<std::is_base_of<Base::Binary, T>::value, void>;

/// Resolves to `void` when `T` is a JSON-format serializer or deserializer (i.e., derives from
/// `Base::JSON`). Use this in the `Enable` parameter of a `CustomClass` partial specialization
/// to restrict the specialization to the JSON backend only.
template <typename T>
using IfJSON = Impl::EnableIfT<std::is_base_of<Base::JSON, T>::value, void>;

/// Resolves to `void` when `T` is an XML-format serializer or deserializer (i.e., derives from
/// `Base::XML`). Use this in the `Enable` parameter of a `CustomClass` partial specialization
/// to restrict the specialization to the XML backend only.
template <typename T>
using IfXML = Impl::EnableIfT<std::is_base_of<Base::XML, T>::value, void>;

/// Resolves to `void` when `T` is *not* a binary-format serializer or deserializer. Use this to
/// write a `CustomClass` specialization that applies to all backends except binary.
template <typename T>
using IfNotBinary = Impl::EnableIfT<!std::is_base_of<Base::Binary, T>::value, void>;

/// Resolves to `void` when `T` is *not* a JSON-format serializer or deserializer. Use this to
/// write a `CustomClass` specialization that applies to all backends except JSON.
template <typename T>
using IfNotJSON = Impl::EnableIfT<!std::is_base_of<Base::JSON, T>::value, void>;

/// Resolves to `void` when `T` is *not* an XML-format serializer or deserializer. Use this to
/// write a `CustomClass` specialization that applies to all backends except XML.
template <typename T>
using IfNotXML = Impl::EnableIfT<!std::is_base_of<Base::XML, T>::value, void>;
}  // namespace Serio
