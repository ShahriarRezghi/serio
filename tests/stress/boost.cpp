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

#ifdef SERIO_TEST_BOOST
#include <boost/optional/optional_io.hpp>
#include <boost/tuple/tuple_comparison.hpp>

template <typename T>
using BoostArray = boost::array<T, 50>;

template <typename T>
using StaticVector = boost::container::static_vector<T, 100>;

template <typename T>
using SmallVector = boost::container::small_vector<T, 50>;

TEST_101(Type2, BoostArray, BoostArray);
TEST_101(Type2, SmallVector, SmallVector);
TEST_101(Type2, StaticVector, StaticVector);
TEST_101(Type2, BoostVector, boost::container::vector);
TEST_101(Type2, BoostList, boost::container::list);
TEST_101(Type2, BoostSList, boost::container::slist);
TEST_101(Type2, BoostDeque, boost::container::deque);
TEST_101(Type2, BoostDevector, boost::container::devector);
TEST_101(Type2, StableVector, boost::container::stable_vector);

TEST_101(Type1, BoostSet, boost::container::set);
TEST_101(Type1, BoostMultiset, boost::container::multiset);
TEST_101(Type1, FlatSet, boost::container::flat_set);
TEST_101(Type1, FlatMultiset, boost::container::flat_multiset);
TEST_10(Type1, BoostUnorderedset, boost::unordered_set);
TEST_10(Type1, BoostUnorderedMultiset, boost::unordered_multiset);

TEST_201(Type1, BoostMap, boost::container::map);
TEST_20(Type1, BoostMultimap, boost::container::multimap);
TEST_201(Type1, FlatMap, boost::container::flat_map);
TEST_20(Type1, FlatMultimap, boost::container::flat_multimap);
TEST_201(Type1, BoostUnorderedMap, boost::unordered_map);
TEST_201(Type1, BoostUnorderedMultimap, boost::unordered_multimap);

// TEST_10(Type1, Optional, boost::optional);
// TODO
// TEST_30(Type2, Tuple, boost::tuple);
// TEST(Variant, Test) { ProcessAll<boost::variant<bool, char, int, double, boost::container::string>>(); }
// TODO fusion
#endif
