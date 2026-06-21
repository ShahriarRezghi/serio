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

#include "serio.h"

#ifdef SERIO_USE_ENCRYPTION
#include <sodium.h>
#endif

#ifdef SERIO_USE_COMPRESSION
#include <zstd.h>
#endif

namespace Serio
{
namespace Impl
{
#ifdef SERIO_USE_COMPRESSION
void compress(StringView input, std::string& output, int level, Size header)
{
    SERIO_ASSERT(0 <= level && level <= 22, "Compression level must be in the range of 0-22");
    auto size = ZSTD_compressBound(input.size);
    output.resize(header + size);
    auto ptr = (char*)output.data() + header;
    size = ZSTD_compress(ptr, size, input.data, input.size, level);
    SERIO_ASSERT(!ZSTD_isError(size), "Compression failed: " + ZSTD_getErrorName(size));
    output.resize(header + size);
}

void decompress(StringView input, std::string& output)
{
    auto size = ZSTD_getFrameContentSize(input.data, input.size);
    SERIO_ASSERT(size != ZSTD_CONTENTSIZE_ERROR, "Frame content size error");
    SERIO_ASSERT(size != ZSTD_CONTENTSIZE_UNKNOWN, "Frame content size unknown");
    output.resize(size);
    size = ZSTD_decompress((char*)output.data(), size, input.data, input.size);
    SERIO_ASSERT(!ZSTD_isError(size), "Decompression failed: " + ZSTD_getErrorName(size));
    output.resize(size);
}
#endif

#ifdef SERIO_USE_ENCRYPTION
inline void deriveKey(unsigned char* key, Size keySize, const std::string& password, const unsigned char* salt,
                      Size saltSize)
{
    SERIO_ASSERT(key != nullptr, "Key buffer must not be null");
    SERIO_ASSERT(keySize > 0, "Key size must be greater than zero");
    SERIO_ASSERT(!password.empty(), "Password must not be empty");
    SERIO_ASSERT(salt != nullptr, "Salt buffer must not be null");
    SERIO_ASSERT(saltSize > 0, "Salt size must be greater than zero");

    crypto_generichash_state state;

    const int rc = crypto_generichash_init(&state, nullptr, 0, keySize);

    SERIO_ASSERT(rc == 0, "crypto_generichash_init failed");

    SERIO_ASSERT(crypto_generichash_update(&state, reinterpret_cast<const unsigned char*>(password.data()),
                                           password.size()) == 0,
                 "crypto_generichash_update(password) failed");

    SERIO_ASSERT(crypto_generichash_update(&state, salt, saltSize) == 0, "crypto_generichash_update(salt) failed");

    SERIO_ASSERT(crypto_generichash_final(&state, key, keySize) == 0, "crypto_generichash_final failed");
}

void encrypt(StringView input, std::string& output, const std::string& password, Size header)
{
    // libsodium must be initialized once before use
    SERIO_ASSERT(sodium_init() >= 0, "Failed to initialize libsodium");
    SERIO_ASSERT(!password.empty(), "Password must not be empty");

    constexpr Size SALT_SIZE = 32;
    constexpr Size NONCE_SIZE = crypto_secretbox_NONCEBYTES;  // 24
    constexpr Size KEY_SIZE = crypto_secretbox_KEYBYTES;      // 32
    constexpr Size MAC_SIZE = crypto_secretbox_MACBYTES;      // 16

    unsigned char salt[SALT_SIZE];
    unsigned char nonce[NONCE_SIZE];
    randombytes_buf(salt, SALT_SIZE);
    randombytes_buf(nonce, NONCE_SIZE);

    unsigned char key[KEY_SIZE];
    deriveKey(key, KEY_SIZE, password, salt, SALT_SIZE);

    output.resize(header + SALT_SIZE + NONCE_SIZE + MAC_SIZE + input.size);
    auto* ptr = reinterpret_cast<unsigned char*>(output.data()) + header;

    std::memcpy(ptr, salt, SALT_SIZE);
    ptr += SALT_SIZE;
    std::memcpy(ptr, nonce, NONCE_SIZE);
    ptr += NONCE_SIZE;

    int rc = crypto_secretbox_easy(ptr, reinterpret_cast<const unsigned char*>(input.data), input.size, nonce, key);
    SERIO_ASSERT(rc == 0, "Encryption failed");
}

void decrypt(StringView input, std::string& output, const std::string& password)
{
    SERIO_ASSERT(!password.empty(), "Password must not be empty");
    SERIO_ASSERT(sodium_init() >= 0, "Failed to initialize libsodium");

    constexpr Size SALT_SIZE = 32;
    constexpr Size NONCE_SIZE = crypto_secretbox_NONCEBYTES;
    constexpr Size KEY_SIZE = crypto_secretbox_KEYBYTES;
    constexpr Size MAC_SIZE = crypto_secretbox_MACBYTES;
    constexpr Size OVERHEAD = SALT_SIZE + NONCE_SIZE + MAC_SIZE;

    SERIO_ASSERT(input.size > OVERHEAD, "Encrypted data too small");

    const auto* ptr = reinterpret_cast<const unsigned char*>(input.data);

    const unsigned char* salt = ptr;
    ptr += SALT_SIZE;
    const unsigned char* nonce = ptr;
    ptr += NONCE_SIZE;

    unsigned char key[KEY_SIZE];
    deriveKey(key, KEY_SIZE, password, salt, SALT_SIZE);

    Size cipherLen = input.size - SALT_SIZE - NONCE_SIZE;
    output.resize(cipherLen - MAC_SIZE);

    int rc = crypto_secretbox_open_easy(reinterpret_cast<unsigned char*>(output.data()), ptr, cipherLen, nonce, key);
    SERIO_ASSERT(rc == 0, "Decryption failed: wrong password or corrupted data");
}
#endif
}  // namespace Impl
}  // namespace Serio

// struct StreamCompress
// {
//     Buffer input, output;
//     Size insize = 0, outsize = 0;
//     std::shared_ptr<void> context;
//     std::basic_ostream<char>* stream;

//     StreamCompress(std::basic_ostream<char>* stream, int level)
//     {
//         context = std::shared_ptr<void>(ZSTD_createCCtx(), ZSTD_freeCCtx);
//         ZSTD_CCtx_setParameter((ZSTD_CCtx*)context.get(), ZSTD_c_compressionLevel, level);
//         input.resize(ZSTD_CStreamInSize());
//         output.resize(ZSTD_CStreamOutSize());
//     }

//     void _put(bool finish)
//     {
//         auto ctx = (ZSTD_CCtx*)context.get();
//         auto mode = finish ? ZSTD_e_end : ZSTD_e_continue;
//         ZSTD_inBuffer inbuff = {input.data(), insize, 0};

//         while (true)
//         {
//             ZSTD_outBuffer outbuff = {(void*)output.data(), output.size(), 0};
//             size_t const remaining = ZSTD_compressStream2(ctx, &outbuff, &inbuff, mode);
//             SERIO_ASSERT(!ZSTD_isError(remaining), "Compression failed:" << ZSTD_getErrorName(remaining));
//             stream->write(output.data(), outbuff.pos);
//             if (finish ? (remaining == 0) : (inbuff.pos == inbuff.size)) break;
//         }
//     }

//     void put(const char* data, Size length)
//     {
//         while (length > 0)
//         {
//             Size count = std::min(length, input.size() - insize);
//             std::copy(data, data + count, (char*)input.data() + insize);
//             insize += count, length -= count, data += count;
//             if (insize >= input.size()) _put(false);
//         }
//     }

//     void finish() { _put(true); }
// };

// struct StreamDecompress
// {
//     Buffer input, output;
//     Size insize = 0, outsize = 0;
//     std::shared_ptr<void> context;
//     std::basic_istream<char>* stream;

//     StreamDecompress(std::basic_istream<char>* stream, int level)
//     {
//         context = std::shared_ptr<void>(ZSTD_createDCtx(), ZSTD_freeDCtx);
//         input.resize(ZSTD_CStreamInSize());
//         output.resize(ZSTD_CStreamOutSize());
//     }

//     void _get()
//     {
//         auto ctx = (ZSTD_DCtx*)context.get();

//         // TODO
//     }

//     void get(char* data, Size length)
//     {
//         // TODO
//     }
// };

// TODO std::string_view and std::span are incorrectly marked resizable
// TODO maybe remove resize for Array
// TODO make sure empty tuple works
// TODO set maximum-size limits in DOptions
// Size maxContainerSize = ...;
// Size maxStringSize = ...;
// Size maxBytes = ...;

// chrono year_month_day, sys_time, utc_time, zoned_time
// std::filesystem::path
// ranges
// std::expected
// flat_map, flat_set, flat_multimap, flat_multiset
// std::mdspan

// TODO NVP temporary rvalue binding
// TODO weak encryption and wrong password garbage value
