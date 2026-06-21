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

#include "helper.h"

TYPED_TEST(Type2, Basic) { ProcessAll<TypeParam>(); }
TEST_10(Type1, Complex, std::complex);
TEST_20(Type2, Pair, std::pair);
TEST_30(Type2, Tuple, std::tuple);
TEST_10(Type2, ShapredPtr, std::shared_ptr);
TEST_10(Type2, UniquePtr, std::unique_ptr);

#if __cplusplus >= 201703L
TEST_10(Type2, Optional, std::optional);
TEST(Variant, Test) { ProcessAll<std::variant<bool, char, int, double, std::string>>(); }
#endif

TEST(Base64, EncodeDecode)
{
    for (size_t i = 0; i < 1000; ++i)
    {
        std::string input(rand() % 100, 0);
        for (auto &c : input) c = rand() % 256 - 128;

        std::string temp(Serio::Impl::b64encSize(input.size()), 0);
        Serio::Impl::base64Encode((const uint8_t *)input.data(), &temp[0], input.size());

        std::string output(Serio::Impl::b64decSize(temp.data(), temp.size()), 0);
        Serio::Impl::base64Decode((const uint8_t *)temp.data(), (uint8_t *)output.data(), temp.size());

        ASSERT_EQ(input, output);
    }
}

// TODO test std::string_view and std::span
