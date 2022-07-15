# BSD 3-Clause License
#
# Copyright (c) 2019-2020, Shahriar Rezghi <shahriar25.ss@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.5)
project(Serio VERSION 0.4.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SERIO_MASTER OFF CACHE INTERNAL "")
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(SERIO_MASTER ON CACHE INTERNAL "")
endif()

set(SERIO_VERSION ${PROJECT_VERSION} CACHE INTERNAL "")
set(SERIO_LIBRARIES "serio" CACHE INTERNAL "")
set(SERIO_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/" CACHE INTERNAL "")

option(SERIO_BUILD_TESTS "Build Serio Tests" ${SERIO_MASTER})
option(SERIO_INSTALL_LIB "Install Serio Library" ${SERIO_MASTER})

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serio/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/serio/config.h")
file(GLOB HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serio/serio.h"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/serio/config.h")

add_library(serio INTERFACE)
target_include_directories(serio INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/gen/>"
    "$<INSTALL_INTERFACE:include>")

if(SERIO_INSTALL_LIB)
    install(TARGETS serio
        EXPORT serioConfig
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
    install(FILES ${HEADERS}
        DESTINATION include/serio)
    install(EXPORT serioConfig
        NAMESPACE serio::
        DESTINATION lib/cmake/serio)
    export(TARGETS serio
        NAMESPACE serio::
        FILE "${PROJECT_BINARY_DIR}/serioConfig.cmake")
endif()

if(SERIO_BUILD_TESTS)
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif(WIN32)

    include_directories(./src/)
    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(INSTALL_GTEST OFF CACHE BOOL "")
    add_subdirectory(./test/googletest/)

    add_executable(tests ${HEADERS} "test/main.cpp")
    target_link_libraries(tests serio gtest gtest_main)
    add_test(NAME serio_test COMMAND tests)
endif()
