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

#ifdef SERIO_TEST_QT
template <typename T>
using QtArray = QVarLengthArray<T, 50>;

TEST_101(Type2, QVector, QVector);
TEST_101(Type2, QtArray, QtArray);
TEST_101(Type2, QList, QList);
TEST_10(Type1, QSet, QSet);
TEST_101(Type1, QStack, QStack);
TEST_101(Type1, QQueue, QQueue);
TEST_20(Type1, QMap, QMap);
TEST_20(Type1, QHash, QHash);
TEST_20(Type1, QMultiMap, QMultiMap);
TEST_20(Type1, QMultiHash, QMultiHash);
TEST(QStringList, Test) { ProcessAll<QStringList>(); }
#endif
