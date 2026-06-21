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

#include <serio/json.h>
#include <serio/xml.h>

namespace Serio
{
/// Selects the serialization backend. Passed as a compile-time template argument to every public
/// API function. Choosing the format at the call site means a single codebase can write the same
/// data in any format without touching the type definitions.
///
/// - `Binary` produces a compact, versioned byte stream with optional CRC32, zstd compression,
///   and password encryption. It is the fastest format and the smallest on the wire.
/// - `JSON` produces human-readable UTF-8 JSON text. Non-text strings must be wrapped with
///   `binaryString()` to be base64-encoded; complex types map to JSON arrays or objects when
///   `NVP` is used.
/// - `XML` produces UTF-8 XML text with the same structural mapping as JSON. Prefer JSON unless
///   an XML consumer is on the other end.
enum Type
{
    Binary,
    JSON,
    XML,
};

/// Options that control how data is serialized. Pass a default-constructed instance when none of
/// the optional features are needed; only set the fields relevant to your use case.
///
/// Not all combinations are available in all contexts. The `write()` streaming function requires
/// `enableChecksum = false`, `compressLevel = -1`, and `encryptPassword` to be empty, because
/// those features need the full payload to be available before the header can be written.
struct SerializeOptions
{
    /// When true, JSON and XML output is written without indentation or newlines. When false
    /// (the default), the output is pretty-printed. Has no effect on binary output.
    bool compactFrom = false;

    /// When true, a CRC32 checksum of the payload is computed and stored in the binary header,
    /// extending it from 4 bytes to 8 bytes. On deserialization the checksum is recomputed and
    /// compared; a mismatch throws `Serio::Exception`. Has no effect on JSON or XML output, and
    /// is not supported in streaming mode.
    bool enableChecksum = false;

    /// zstd compression level for binary output. The valid range is 0–22, where higher values
    /// produce smaller output at the cost of more CPU time. Set to -1 (the default) to disable
    /// compression. When used together with `encryptPassword`, data is compressed first and then
    /// encrypted. Not supported in streaming mode.
    int compressLevel = -1;  // 0-22,-1=disable

    /// Password for encryption of binary output. An empty string (the default) means no
    /// encryption. When set, the payload is encrypted after any optional compression step. The
    /// same password must be supplied in `DeserializeOptions::decryptPassword` to recover the
    /// data. Not supported in streaming mode.
    std::string encryptPassword;
};

/// Options that control how data is deserialized. Pass a default-constructed instance when none
/// of the optional features are needed.
struct DeserializeOptions
{
    /// Maximum number of elements the deserializer will accept for any single container (vector,
    /// string, map, etc.). If the serialized length field exceeds this value, `Serio::Exception`
    /// is thrown. Set to 0 (the default) to impose no limit. Use a non-zero value when reading
    /// untrusted data to prevent a maliciously crafted size field from triggering a huge
    /// allocation.
    Size maxLength = 0;

    /// Password for decrypting binary data that was serialized with `SerializeOptions::encryptPassword`.
    /// Must match the password used during serialization; a mismatch produces garbage output or
    /// an exception depending on the underlying crypto library. Leave empty (the default) when
    /// the data is not encrypted.
    std::string decryptPassword;
};

namespace Impl
{
struct Flags
{
    enum : uint8_t
    {
        None = 0b00000000,
        Checksum = 0b00000001,
        Compress = 0b00000010,
        Encrypt = 0b00000100,
    };
};

inline void writeHeader(char* data, bool checksum, bool compress, bool encrypt, uint32_t crc)
{
    uint8_t flags = Flags::None;
    if (checksum) flags |= Flags::Checksum;
    if (compress) flags |= Flags::Compress;
    if (encrypt) flags |= Flags::Encrypt;

    data[0] = SERIO_VERSION_MAJOR;
    data[1] = SERIO_VERSION_MINOR;
    data[2] = flags;
    data[3] = 0;

    if (!checksum) return;
    BasicType::serialize(data + 4, crc);
}

inline void writeHeader(std::ostream& stream)
{
    char data[4];
    data[0] = SERIO_VERSION_MAJOR;
    data[1] = SERIO_VERSION_MINOR;
    data[2] = 0;
    data[3] = 0;
    stream.write(data, 4);
}

inline void readHeader(StringView data, uint8_t& flags, Size& header)
{
    SERIO_ASSERT(data.size >= 4, "Data size must be at least 4 bytes");
    uint8_t major = data[0];
    uint8_t minor = data[1];
    flags = data[2];
    header = 4;

    SERIO_ASSERT(major == SERIO_VERSION_MAJOR, "Major version mismatch");
    SERIO_ASSERT(minor <= SERIO_VERSION_MINOR, "Minor version mismatch");
    SERIO_ASSERT((flags & 0b11111000) == 0, "Invalid flags in header");
    SERIO_ASSERT(data[3] == 0, "Invalid data at position 3 in header");

    if (flags & Flags::Checksum)
    {
        SERIO_ASSERT(data.size >= 8, "Data size must be at least 8 bytes for checksum");
        uint32_t crc = 0;
        BasicType::deserialize(data.data + 4, crc);
        SERIO_ASSERT(crc == crcCreate(data.view(8)), "CRC check failed");
        header += 4;
    }
}

inline void readHeader(std::istream& stream, uint8_t& flags)
{
    // TODO check if stream can read the bytes
    char data[4];
    stream.read(data, 4);

    uint8_t major = data[0];
    uint8_t minor = data[1];
    flags = data[2];

    SERIO_ASSERT(major == SERIO_VERSION_MAJOR, "Major version mismatch");
    SERIO_ASSERT(minor <= SERIO_VERSION_MINOR, "Minor version mismatch");
    SERIO_ASSERT(flags == 0, "Invalid flags in header");
    SERIO_ASSERT(data[3] == 0, "Invalid data at position 3 in header");
}

template <Type T, typename... Ts>
EnableIfT<T == Type::Binary, std::string> serialize(const SerializeOptions& options, Ts&&... ts)
{
    bool checksum = options.enableChecksum;
    bool compress = options.compressLevel > -1;
    bool encrypt = !options.encryptPassword.empty();

    std::string data, buffer;
    Size headerSize = 4 + (checksum ? 4 : 0);

    if (compress && !encrypt)
    {
        Impl::Serializer(buffer).process(std::forward<Ts>(ts)...);
        Impl::compress(buffer, data, options.compressLevel, headerSize);
    }
    else if (!compress && encrypt)
    {
        Impl::Serializer(buffer).process(std::forward<Ts>(ts)...);
        Impl::encrypt(buffer, data, options.encryptPassword, headerSize);
    }
    else if (compress && encrypt)
    {
        Impl::Serializer(data).process(std::forward<Ts>(ts)...);
        Impl::compress(data, buffer, options.compressLevel, 0);
        Impl::encrypt(buffer, data, options.encryptPassword, headerSize);
    }
    else
    {
        data.resize(headerSize);
        Impl::Serializer(data).process(std::forward<Ts>(ts)...);
    }

    uint32_t crc = 0;
    if (checksum) crc = Impl::crcCreate(StringView(data).view(headerSize));
    Impl::writeHeader(&data.front(), checksum, compress, encrypt, crc);
    return data;
}

template <Type T, typename... Ts>
EnableIfT<T == Type::JSON, std::string> serialize(const SerializeOptions& options, Ts&&... ts)
{
    JsonSerializer::Document document;
    return JsonSerializer(document.GetAllocator())  //
        .process(std::forward<Ts>(ts)...)
        .finalize(options.compactFrom);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::XML, std::string> serialize(const SerializeOptions& options, Ts&&... ts)
{
    XmlSerializer::Document document;
    return XmlSerializer(document, &document)  //
        .process(std::forward<Ts>(ts)...)
        .finalize(options.compactFrom);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::Binary, Size> deserialize(const DeserializeOptions& options, StringView data, Ts&&... ts)
{
    uint8_t flags;
    Size header = 0;
    std::string buffer;

    Impl::readHeader(data, flags, header);
    bool compress = flags & Impl::Flags::Compress;
    bool encrypt = flags & Impl::Flags::Encrypt;
    auto ptr = data.data + header;
    auto size = data.size - header;

    if (compress && !encrypt)
    {
        Impl::decompress(data.view(header), buffer);
        ptr = &buffer.front();
        size = buffer.size();
    }
    else if (!compress && encrypt)
    {
        Impl::decrypt(data.view(header), buffer, options.decryptPassword);
        ptr = &buffer.front();
        size = buffer.size();
    }
    else if (compress && encrypt)
    {
        std::string temp;
        Impl::decrypt(data.view(header), temp, options.decryptPassword);
        Impl::decompress(temp, buffer);
        ptr = &buffer.front();
        size = buffer.size();
    }

    return Impl::Deserializer(ptr, size, options.maxLength).process(std::forward<Ts>(ts)...).progress();
}

template <Type T, typename... Ts>
EnableIfT<T == Type::JSON, void> deserialize(const DeserializeOptions& options, StringView data, Ts&&... ts)
{
    JsonDeserializer::Document document;
    JsonDeserializer::parse(document, data).process(std::forward<Ts>(ts)...);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::XML, void> deserialize(const DeserializeOptions& options, StringView data, Ts&&... ts)
{
    XmlDeserializer::Document document;
    XmlDeserializer::parse(document, data).process(std::forward<Ts>(ts)...);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::Binary, void> write(const SerializeOptions& options, std::ostream& stream, Ts&&... ts)
{
    Impl::StreamSerializer(stream).process(std::forward<Ts>(ts)...);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::JSON, void> write(const SerializeOptions& options, std::ostream& stream, Ts&&... ts)
{
    JsonSerializer::Document document;
    JsonSerializer(document.GetAllocator()).process(std::forward<Ts>(ts)...).finalize(stream, options.compactFrom);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::XML, void> write(const SerializeOptions& options, std::ostream& stream, Ts&&... ts)
{
    XmlSerializer::Document document;
    XmlSerializer(document, &document).process(std::forward<Ts>(ts)...).finalize(stream, options.compactFrom);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::Binary, void> read(const DeserializeOptions& options, std::istream& stream, Ts&&... ts)
{
    Impl::StreamDeserializer deserializer(stream, options.maxLength);
    deserializer.process(std::forward<Ts>(ts)...);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::JSON, void> read(const DeserializeOptions& options, std::istream& stream, Ts&&... ts)
{
    JsonDeserializer::Document document;
    JsonDeserializer::parse(document, stream).process(std::forward<Ts>(ts)...);
}

template <Type T, typename... Ts>
EnableIfT<T == Type::XML, void> read(const DeserializeOptions& options, std::istream& stream, Ts&&... ts)
{
    std::string buffer;
    XmlDeserializer::Document document;
    XmlDeserializer::parse(document, stream, buffer).process(std::forward<Ts>(ts)...);
}
}  // namespace Impl

/// Serializes one or more values into an in-memory `std::string` using the format selected by
/// `type`. When multiple values are passed they are serialized as an ordered sequence — a flat
/// byte stream in binary mode, or a JSON/XML array when using those formats.
///
/// For binary output, the returned string begins with a 4-byte header (8 bytes when
/// `options.enableChecksum` is true) that records the library version and which optional features
/// are active. The payload follows immediately. Compression and encryption, when enabled, are
/// applied in that order: compress first, then encrypt.
///
/// For JSON and XML output, `options.enableChecksum`, `options.compressLevel`, and
/// `options.encryptPassword` are ignored.
///
/// @tparam type  The serialization format. One of `Type::Binary`, `Type::JSON`, `Type::XML`.
/// @tparam Ts    Deduced from the values passed; any serializable type is accepted.
/// @param options  Serialization options controlling checksum, compression, encryption, and
///                 text formatting.
/// @param ts       One or more values to serialize.
/// @returns A `std::string` containing the full serialized representation, including any header.
template <Type type, typename... Ts>
std::string serialize(const SerializeOptions& options, Ts&&... ts)
{
    return Impl::serialize<type>(options, std::forward<Ts>(ts)...);
}

/// Deserializes one or more values from the byte buffer `data` using the format selected by
/// `type`. The values are filled in the same order they were passed to `serialize`. The buffer
/// must begin with a valid Serio header (for binary) or be a complete JSON/XML document; passing
/// a truncated or corrupted buffer throws `Serio::Exception`.
///
/// For binary data the header is parsed first to determine which optional features were active.
/// Decryption is applied before decompression if both flags are set. The `options.maxLength`
/// guard is enforced against every container size field encountered in the payload.
///
/// @tparam type  The serialization format matching the one used when the data was produced.
/// @tparam Ts    Deduced from the output variables passed; any deserializable type is accepted.
/// @param options  Deserialization options controlling the maximum container size and decryption
///                 password.
/// @param data     A `StringView` over the complete serialized buffer, including the header.
/// @param ts       One or more output variables to fill; must match the types and order used
///                 during serialization.
template <Type type, typename... Ts>
void deserialize(const DeserializeOptions& options, StringView data, Ts&&... ts)
{
    Impl::deserialize<type>(options, data, std::forward<Ts>(ts)...);
}

/// Serializes one or more values and writes the result to the file at `path`, creating or
/// truncating the file. This is a convenience wrapper around `serialize()` followed by a file
/// write; the full serialized buffer is held in memory before writing. Throws `Serio::Exception`
/// if the file cannot be opened for writing.
///
/// All `SerializeOptions` fields are respected, including checksum, compression, and encryption.
///
/// @tparam type  The serialization format.
/// @param options  Serialization options.
/// @param path     Filesystem path to the output file.
/// @param ts       One or more values to serialize.
template <Type type, typename... Ts>
void save(const SerializeOptions& options, const std::string& path, Ts&&... ts)
{
    auto data = serialize<type>(options, std::forward<Ts>(ts)...);
    SERIO_ASSERT(Impl::writeFile(path, data), "Failed to open file for writing");
}

/// Reads the file at `path` and deserializes one or more values from its contents. On Unix-like
/// platforms the file is memory-mapped with `mmap` to avoid reading the entire file into a heap
/// buffer; on other platforms it is read into a `std::string` first. Throws `Serio::Exception`
/// if the file cannot be opened or if the contents are invalid.
///
/// All `DeserializeOptions` fields are respected, including the `maxLength` guard and the
/// decryption password.
///
/// @tparam type  The serialization format matching the one used when the file was produced.
/// @param options  Deserialization options.
/// @param path     Filesystem path to the input file.
/// @param ts       One or more output variables to fill.
template <Type type, typename... Ts>
void load(const DeserializeOptions& options, const std::string& path, Ts&&... ts)
{
#ifdef SERIO_UNIX
    Impl::MappedFile file;
    SERIO_ASSERT(file.open(path), "Failed to open file for reading");
    deserialize<type>(options, file.view(), std::forward<Ts>(ts)...);
#else
    std::string data;
    SERIO_ASSERT(Impl::readFile(path, data), "Failed to open file for reading");
    deserialize<type>(options, data, std::forward<Ts>(ts)...);
#endif
}

/// Serializes one or more values and writes the output to `stream`. For binary output the values
/// are written directly to the stream as they are processed, with a 4-byte header prepended; no
/// full-payload buffer is held in memory. For JSON and XML the document is accumulated internally
/// and flushed to the stream in one shot once all values are processed.
///
/// Checksum, compression, and encryption are not supported in streaming mode because they all
/// require the full payload to be known before the header can be finalized. Enabling any of
/// these in `options` throws `Serio::Exception`. Use `serialize()` + `save()` when those
/// features are needed.
///
/// @tparam type  The serialization format.
/// @param options  Serialization options. `enableChecksum` must be false, `compressLevel` must
///                 be -1, and `encryptPassword` must be empty.
/// @param stream   The output stream to write to.
/// @param ts       One or more values to serialize.
template <Type type, typename... Ts>
void write(const SerializeOptions& options, std::ostream& stream, Ts&&... ts)
{
    SERIO_ASSERT(!options.enableChecksum, "Checksum is not supported in stream mode");
    SERIO_ASSERT(options.compressLevel < 0, "Compression is not supported in stream mode");
    SERIO_ASSERT(options.encryptPassword.empty(), "Encryption is not supported in stream mode");
    if (type == Type::Binary) Impl::writeHeader(stream);
    Impl::write<type>(options, stream, std::forward<Ts>(ts)...);
}

/// Reads and deserializes one or more values from `stream`. For binary streams, the 4-byte
/// header is consumed and validated first; if the header flags indicate that the data was
/// compressed or encrypted, `Serio::Exception` is thrown because those features are not
/// supported in streaming mode. For JSON and XML, the entire stream content is parsed before
/// any values are extracted.
///
/// @tparam type  The serialization format matching the one used when writing to the stream.
/// @param options  Deserialization options. Only `maxLength` is relevant; `decryptPassword` is
///                 not used because encryption is not supported in streaming mode.
/// @param stream   The input stream to read from.
/// @param ts       One or more output variables to fill.
template <Type type, typename... Ts>
void read(const DeserializeOptions& options, std::istream& stream, Ts&&... ts)
{
    if (type == Type::Binary)
    {
        uint8_t flags;
        Impl::readHeader(stream, flags);
        bool compress = flags & Impl::Flags::Compress;
        bool encrypt = flags & Impl::Flags::Encrypt;
        SERIO_ASSERT(!compress, "Decompression is not supported in stream mode");
        SERIO_ASSERT(!encrypt, "Decryption is not supported in stream mode");
    }
    Impl::read<type>(options, stream, std::forward<Ts>(ts)...);
}
}  // namespace Serio
