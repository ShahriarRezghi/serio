// /*
//   Copyright (c) 2014, Randolph Voorhies, Shane Grant
//   All rights reserved.

//   Redistribution and use in source and binary forms, with or without
//   modification, are permitted provided that the following conditions are met:
//       * Redistributions of source code must retain the above copyright
//         notice, this list of conditions and the following disclaimer.
//       * Redistributions in binary form must reproduce the above copyright
//         notice, this list of conditions and the following disclaimer in the
//         documentation and/or other materials provided with the distribution.
//       * Neither the name of the copyright holder nor the
//         names of its contributors may be used to endorse or promote products
//         derived from this software without specific prior written permission.

//   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
//   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// */
//
// #include "polymorphic.h"

// TEST_SUITE_BEGIN("polymorphic");

// TEST_CASE("binary_polymorphic")
// {
//   test_polymorphic<TestDeserializer<Serio::Binary>, TestSerializer<Serio::Binary>>();
// }

// TEST_CASE("xml_polymorphic")
// {
//   test_polymorphic<TestDeserializer<Serio::XML>, TestSerializer<Serio::XML>>();
// }

// TEST_CASE("json_polymorphic")
// {
//   test_polymorphic<TestDeserializer<Serio::JSON>, TestSerializer<Serio::JSON>>();
// }

// #if CEREAL_THREAD_SAFE
// TEST_CASE("binary_polymorphic_threading")
// {
//   test_polymorphic_threading<TestDeserializer<Serio::Binary>, TestSerializer<Serio::Binary>>();
// }

// TEST_CASE("portable_binary_polymorphic_threading")
// {
//   test_polymorphic_threading<TestDeserializer<Serio::Binary>, TestSerializer<Serio::Binary>>();
// }

// TEST_CASE("json_polymorphic_threading")
// {
//   test_polymorphic_threading<TestDeserializer<Serio::JSON>, TestSerializer<Serio::JSON>>();
// }
// #endif // CEREAL_THREAD_SAFE

// TEST_SUITE_END();
