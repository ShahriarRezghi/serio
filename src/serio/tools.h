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

#include <serio/base.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <codecvt>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <forward_list>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <vector>
//
#if SERIO_CPP_VERSION >= 201703L
#include <optional>
#include <string_view>
#endif
//
#if SERIO_CPP_VERSION >= 202002L
#include <span>
#endif
//
// #if SERIO_CPP_VERSION >= 202302L
// #include <flat_map>
// #include <flat_multimap>
// #include <flat_set>
// #include <float_multiset>
// #endif

#ifdef SERIO_UNIX
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace Serio
{
namespace Impl
{
#ifdef SERIO_UNIX
struct MappedFile
{
    bool open(const std::string& path)
    {
        file = ::open(path.c_str(), O_RDONLY);
        if (file == -1) return false;
        struct stat stats;
        if (fstat(file, &stats) == -1) return false;
        if (stats.st_size == 0) return false;
        auto addr = mmap(NULL, stats.st_size, PROT_READ, MAP_PRIVATE, file, 0);
        if (addr == MAP_FAILED) return false;
        data = StringView((const char*)addr, stats.st_size);
        return true;
    }

    ~MappedFile()
    {
        if (data.data) munmap(const_cast<char*>(data.data), data.size);
        if (file != -1) close(file);
    }

    StringView view() { return data; }

private:
    int file{-1};
    StringView data{nullptr, Size(0)};
};
#endif

inline bool readFile(const std::string& path, std::string& data)
{
    struct CloseWrapper
    {
        FILE* file;
        ~CloseWrapper() { fclose(file); }
    };

    auto file = fopen(path.c_str(), "rb");
    if (!file) return false;
    CloseWrapper wrapper{file};

    if (fseek(file, 0, SEEK_END) != 0) return false;
    auto size = ftell(file);
    if (size <= 0) return false;
    rewind(file);

    data.resize(size);
    return std::fread((void*)data.data(), 1, size, file) == size;
}

inline bool writeFile(const std::string& path, const std::string& data)
{
    FILE* file = fopen(path.c_str(), "wb");
    if (!file) return false;
    auto written = fwrite(data.data(), 1, data.size(), file);
    fclose(file);
    return written == data.size();
}

inline bool _littleEndian()
{
    uint32_t value = 0x01020304;
    char data[4] = {4, 3, 2, 1}, *list = (char*)&value;
    return data[0] == list[0] && data[1] == list[1] && data[2] == list[2] && data[3] == list[3];
}

inline bool littleEndian()
{
    static bool little = _littleEndian();
    return little;
}

inline Size b64encSize(Size size) { return (size + 2) / 3 * 4; }

inline void base64Encode(const uint8_t* input, char* output, Size size)
{
    const static char table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

    auto convert = [&](uint32_t _1, uint32_t _2, uint32_t _3, char* ptr) {
        uint32_t item = (_1 << 16) | (_2 << 8) | _3;
        ptr[0] = table[(item >> 18) & 0x3F];
        ptr[1] = table[(item >> 12) & 0x3F];
        ptr[2] = table[(item >> 6) & 0x3F];
        ptr[3] = table[item & 0x3F];
    };

    Size i = 0, o = 0;
    for (Size _ = 0; _ < size / 3; ++_, i += 3, o += 4)  //
        convert(input[i], input[i + 1], input[i + 2], output + o);

    if (i >= size) return;
    convert(input[i], i + 1 < size ? input[i + 1] : 0, 0, output + o);
    if (size - i == 1) output[o + 2] = '=';
    output[o + 3] = '=';
}

inline Size b64decSize(const char* input, Size size)
{
    SERIO_ASSERT(size % 4 == 0, "Invalid base64 string encountered when decoding");

    Size len = size / 4 * 3;
    if (size > 0 && input[size - 2] == '=') --len;
    if (size > 0 && input[size - 1] == '=') --len;
    return len;
}

inline void base64Decode(const uint8_t* input, uint8_t* output, Size size)
{
    const static uint8_t table[] = {
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x3E, 0x80, 0x80, 0x80, 0x3F, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
        0x3A, 0x3B, 0x3C, 0x3D, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24,
        0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
        0x80, 0x80, 0x80, 0x80};

    auto convert = [&](uint8_t __1, uint8_t __2, uint8_t __3, uint8_t __4, uint8_t* ptr) {
        uint32_t _1 = table[__1], _2 = table[__2], _3 = table[__3], _4 = table[__4];
        bool wrong = (_1 | _2 | _3 | _4) & uint32_t(0x80);
        SERIO_ASSERT(!wrong, "Invalid base64 string encountered when decoding");
        uint32_t item = (_1 << 18) | (_2 << 12) | (_3 << 6) | _4;
        ptr[0] = (item >> 16) & 0xFF;
        ptr[1] = (item >> 8) & 0xFF;
        ptr[2] = item & 0xFF;
    };

    SERIO_ASSERT(size % 4 == 0, "Invalid base64 string encountered when decoding");
    if (size > 0 && input[size - 1] == '=') --size;
    if (size > 0 && input[size - 1] == '=') --size;

    Size i = 0, o = 0;
    for (Size _ = 0; _ < size / 4; ++_, i += 4, o += 3)  //
        convert(input[i], input[i + 1], input[i + 2], input[i + 3], output + o);

    if (i >= size) return;
    uint8_t temp[3] = {0, 0, 0};
    convert(input[i], input[i + 1], i + 2 < size ? input[i + 2] : 'A', 'A', temp);
    output[o] = temp[0];
    if (size - i == 3) output[o + 1] = temp[1];
}

inline std::string toutf8(const char* first, const char* last)
{
    return std::string(first, last);  //
}
template <typename T>
std::string toutf8(const T* first, const T* last)
{
    std::wstring_convert<std::codecvt_utf8<T>, T> convert;
    return convert.to_bytes(first, last);
}

template <typename... Ts>
void fromutf8(const std::string& input, std::basic_string<char, Ts...>& output)
{
    output = input;
}
template <typename T, typename... Ts>
void fromutf8(const std::string& input, std::basic_string<T, Ts...>& output)
{
    std::wstring_convert<std::codecvt_utf8<T>, T> convert;
    output = convert.from_bytes(input);
}

inline uint32_t _crcCreate(const uint8_t* data, Size size)
{
    const static uint32_t table[] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,  //
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,  //
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,  //
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,  //
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,  //
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,  //
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,  //
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,  //
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,  //
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,  //
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,  //
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,  //
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,  //
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,  //
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,  //
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,  //
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,  //
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,  //
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,  //
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,  //
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,  //
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,  //
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,  //
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,  //
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,  //
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,  //
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,  //
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,  //
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,  //
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,  //
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,  //
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,  //
    };

    uint32_t crc = 0xFFFFFFFF;
    for (Size i = 0; i < size; ++i)  //
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return ~crc;
}

inline uint32_t crcCreate(StringView input)  //
{
    return _crcCreate((const uint8_t*)input.data, input.size);
}

#ifdef SERIO_USE_COMPRESSION
SERIO_API void compress(StringView input, std::string& output, int level, Size header);
SERIO_API void decompress(StringView input, std::string& output);
#else
inline void compress(StringView input, std::string& output, int level, Size header)
{
    SERIO_ASSERT(false, "Compression is not enabled in this build of Serio");
}
inline void decompress(StringView input, std::string& output)
{
    SERIO_ASSERT(false, "Compression is not enabled in this build of Serio");
}
#endif

#ifdef SERIO_USE_ENCRYPTION
SERIO_API void encrypt(StringView input, std::string& output, const std::string& password, Size header);
SERIO_API void decrypt(StringView input, std::string& output, const std::string& password);
#else
inline void encrypt(StringView input, std::string& output, const std::string& password, Size header)
{
    SERIO_ASSERT(false, "Encryption is not enabled in this build of Serio");
}
inline void decrypt(StringView input, std::string& output, const std::string& password)
{
    SERIO_ASSERT(false, "Encryption is not enabled in this build of Serio");
}
#endif

template <typename T>
Size containerSize(const T& value)
{
    return Size(value.size());
}
template <typename... Ts>
Size containerSize(const std::forward_list<Ts...>& value)
{
    return Size(std::distance(value.begin(), value.end()));
}

template <Size I>
struct TypeMatch
{
    using Type = void;
#ifdef __SIZEOF_INT128__
    static_assert(I == 1 || I == 2 || I == 4 || I == 8 || I == 16, "Arithmetic type not supported.");
#else
    static_assert(I == 1 || I == 2 || I == 4 || I == 8, "Arithmetic type not supported.");
#endif
};
template <>
struct TypeMatch<1>
{
    using Type = uint8_t;
};
template <>
struct TypeMatch<2>
{
    using Type = uint16_t;
};
template <>
struct TypeMatch<4>
{
    using Type = uint32_t;
};
template <>
struct TypeMatch<8>
{
    using Type = uint64_t;
};
#ifdef __SIZEOF_INT128__
template <>
struct TypeMatch<16>
{
    using Type = unsigned __int128;
};
#endif

template <size_t I>
struct _BasicType
{
    template <typename T>
    static void serialize(uint8_t* ptr, const T& data)
    {
        _BasicType<I - 8>::serialize(ptr - 1, data);
        *ptr = uint8_t(data >> I);
    }
    template <typename T>
    static void deserialize(const uint8_t* ptr, T& data)
    {
        _BasicType<I - 8>::deserialize(ptr - 1, data);
        data |= T(*ptr) << I;
    }
};
template <>
struct _BasicType<0>
{
    template <typename T>
    static void serialize(uint8_t* ptr, const T& data)
    {
        *ptr = uint8_t(data);
    }
    template <typename T>
    static void deserialize(const uint8_t* ptr, T& data)
    {
        data |= T(*ptr);
    }
};
struct BasicType
{
    template <typename T>
    static EnableIfT<!std::is_floating_point<T>::value, void> serialize(char* ptr, const T& data)
    {
        using Type = typename TypeMatch<sizeof(T)>::Type;
        _BasicType<(sizeof(T) - 1) * 8>::serialize((uint8_t*)ptr + sizeof(T) - 1, static_cast<Type>(data));
    }
    template <typename T>
    static EnableIfT<!std::is_floating_point<T>::value, void> deserialize(const char* ptr, T& data)
    {
        using Type = typename TypeMatch<sizeof(T)>::Type;
        Type value = 0;
        _BasicType<(sizeof(T) - 1) * 8>::deserialize((const uint8_t*)ptr + sizeof(T) - 1, value);
        data = static_cast<T>(value);
    }

    template <typename T>
    static EnableIfT<std::is_floating_point<T>::value, void> serialize(char* ptr, const T& data)
    {
        static_assert(std::numeric_limits<T>::is_iec559, "Only IEEE 754 floating point type is supported.");
        using Type = typename TypeMatch<sizeof(T)>::Type;
        Type value;
        std::memcpy(&value, &data, sizeof(T));
        _BasicType<(sizeof(T) - 1) * 8>::serialize((uint8_t*)ptr + sizeof(T) - 1, value);
    }
    template <typename T>
    static EnableIfT<std::is_floating_point<T>::value, void> deserialize(const char* ptr, T& data)
    {
        static_assert(std::numeric_limits<T>::is_iec559, "Only IEEE 754 floating point type is supported.");
        using Type = typename TypeMatch<sizeof(T)>::Type;
        Type value = 0;
        _BasicType<(sizeof(T) - 1) * 8>::deserialize((const uint8_t*)ptr + sizeof(T) - 1, value);
        std::memcpy(&data, &value, sizeof(T));
    }
};

struct Bitset
{
    template <typename T>
    static void serialize(char* data, const T& value)
    {
        std::fill(data, data + size_t((value.size() + 7) / 8), 0);
        for (size_t i = 0; i < value.size(); ++i) data[i / 8] |= value[i] << (i % 8);
    }
    template <typename T>
    static void deserialize(const char* data, T& value)
    {
        for (size_t i = 0; i < value.size(); ++i) value[i] = data[i / 8] & (1 << (i % 8));
    }
};

template <size_t I>
struct Tuple
{
    template <typename Serializer, typename... Ts>
    static void serialize(Serializer& item, const std::tuple<Ts...>& value)
    {
        Tuple<I - 1>::serialize(item, value);
        item << std::get<I - 1>(value);
    }
    template <typename Deserializer, typename... Ts>
    static void deserialize(Deserializer& item, std::tuple<Ts...>& value)
    {
        Tuple<I - 1>::deserialize(item, value);
        item >> std::get<I - 1>(value);
    }
};
template <>
struct Tuple<1>
{
    template <typename Serializer, typename... Ts>
    static void serialize(Serializer& item, const std::tuple<Ts...>& value)
    {
        item << std::get<0>(value);
    }
    template <typename Deserializer, typename... Ts>
    static void deserialize(Deserializer& item, std::tuple<Ts...>& value)
    {
        item >> std::get<0>(value);
    }
};
template <>
struct Tuple<0>
{
    template <typename Serializer, typename... Ts>
    static void serialize(Serializer& item, const std::tuple<Ts...>& value)
    {
    }
    template <typename Deserializer, typename... Ts>
    static void deserialize(Deserializer& item, std::tuple<Ts...>& value)
    {
    }
};

template <typename Tuple, size_t I>
struct Variant
{
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, size_t index, T& value)
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
    template <typename Deserializer, typename T>
    static void deserialize(Deserializer& item, size_t index, T& value)
    {
        if (index == 0)
        {
            typename std::tuple_element<0, Tuple>::type elem;
            item >> elem;
            value = std::move(elem);
        }
    }
};

template <typename T>
struct ValueType
{
    using Type = typename T::value_type;
};
template <typename K, typename T, typename... Ts>
struct ValueType<std::map<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<std::unordered_map<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<std::multimap<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};
template <typename K, typename T, typename... Ts>
struct ValueType<std::unordered_multimap<K, T, Ts...>>
{
    using Type = std::pair<K, T>;
};

// #if SERIO_CPP_VERSION >= 202302L
// template <typename K, typename T, typename... Ts>
// struct ValueType<std::flat_map<K, T, Ts...>>
// {
//     using Type = std::pair<K, T>;
// };
// template <typename K, typename T, typename... Ts>
// struct ValueType<std::flat_multimap<K, T, Ts...>>
// {
//     using Type = std::pair<K, T>;
// };
// #endif

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

template <typename T>
struct IsContinuous : std::false_type
{
};
template <typename... Ts>
struct IsContinuous<std::vector<bool, Ts...>> : std::false_type
{
};
template <typename T, typename... Ts>
struct IsContinuous<std::vector<T, Ts...>> : std::is_arithmetic<T>
{
};
template <typename T, typename... Ts>
struct IsContinuous<std::basic_string<T, Ts...>> : std::is_arithmetic<T>
{
};
template <typename T, size_t N>
struct IsContinuous<std::array<T, N>> : std::is_arithmetic<T>
{
};
template <typename T>
struct IsContinuous<PointerView<T>> : std::is_arithmetic<T>
{
};
template <typename T, size_t N>
struct IsContinuous<StaticArrayView<T, N>> : std::is_arithmetic<T>
{
};

#if SERIO_CPP_VERSION >= 201703L
template <typename T, typename... Ts>
struct IsContinuous<std::basic_string_view<T, Ts...>> : std::is_arithmetic<T>
{
};
#endif

#if SERIO_CPP_VERSION >= 202002L
template <typename T, size_t S>
struct IsContinuous<std::span<T, S>> : std::is_arithmetic<T>
{
};
#endif

template <typename T>
struct IsFixed : std::false_type
{
    static constexpr size_t size() { return 0; }
};
template <typename T, size_t N>
struct IsFixed<std::array<T, N>> : std::true_type
{
    static constexpr size_t size() { return N; }
};
template <typename T, size_t N>
struct IsFixed<StaticArrayView<T, N>> : std::true_type
{
    static constexpr size_t size() { return N; }
};

#if SERIO_CPP_VERSION >= 202002L
template <typename T, size_t S>
struct IsFixed<std::span<T, S>> : std::true_type
{
    static constexpr size_t size() { return S; }
};
template <typename T>
struct IsFixed<std::span<T, std::dynamic_extent>> : std::false_type
{
};
#endif

template <typename T>
struct IsFixedContinuous : std::false_type
{
};
template <typename T, size_t N>
struct IsFixedContinuous<std::array<T, N>> : std::is_arithmetic<T>
{
};
template <typename T, size_t N>
struct IsFixedContinuous<StaticArrayView<T, N>> : std::is_arithmetic<T>
{
};

#if SERIO_CPP_VERSION >= 202002L
template <typename T, size_t S>
struct IsFixedContinuous<std::span<T, S>> : std::is_arithmetic<T>
{
};
template <typename T>
struct IsFixedContinuous<std::span<T, std::dynamic_extent>> : std::false_type
{
};
#endif

template <typename T>
struct IsResizable : std::false_type
{
};
template <typename... Ts>
struct IsResizable<std::vector<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<std::list<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<std::deque<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<std::forward_list<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<std::valarray<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<std::basic_string<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsResizable<PointerView<Ts...>> : std::true_type
{
};

#if SERIO_CPP_VERSION >= 201703L
template <typename... Ts>
struct IsResizable<std::basic_string_view<Ts...>> : std::true_type
{
};
#endif

#if SERIO_CPP_VERSION >= 202002L
template <typename T, size_t S>
struct IsResizable<std::span<T, S>> : std::true_type
{
};
template <typename T>
struct IsResizable<std::span<T, std::dynamic_extent>> : std::true_type
{
};
#endif

template <typename T>
struct IsAppendable : std::false_type
{
};
template <typename... Ts>
struct IsAppendable<std::set<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<std::unordered_set<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<std::multiset<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<std::unordered_multiset<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<std::map<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<std::unordered_map<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<std::multimap<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsAppendable<std::unordered_multimap<Ts...>> : std::true_type
{
};

// #if SERIO_CPP_VERSION >= 202302L
// template <typename... Ts>
// struct IsAppendable<std::flat_set<Ts...>> : std::true_type
// {
// };
// template <typename... Ts>
// struct IsAppendable<std::flat_map<Ts...>> : std::true_type
// {
// };
// template <typename... Ts>
// struct IsAppendable<std::flat_multiset<Ts...>> : std::true_type
// {
// };
// template <typename... Ts>
// struct IsAppendable<std::flat_multimap<Ts...>> : std::true_type
// {
// };
// #endif

template <typename T>
struct IsPointer : std::false_type
{
};
template <typename... Ts>
struct IsPointer<std::shared_ptr<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsPointer<std::unique_ptr<Ts...>> : std::true_type
{
};
template <typename... Ts>
struct IsPointer<std::weak_ptr<Ts...>> : std::true_type
{
};

template <typename T>
struct IsOptional : std::false_type
{
};
#if SERIO_CPP_VERSION >= 201703L
template <typename... Ts>
struct IsOptional<std::optional<Ts...>> : std::true_type
{
};
#endif
}  // namespace Impl
}  // namespace Serio
