# BSD 3-Clause License
#
# Copyright (c) 2019-2026, Shahriar Rezghi <shahriar.rezghi.sh@gmail.com>
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

cmake_minimum_required(VERSION 3.21)
project(Serio VERSION 1.0.0 LANGUAGES CXX)
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
option(SERIO_ENABLE_COMPRESSION "Enable Compression functionallity" OFF)
option(SERIO_ENABLE_ENCRYPTION "Enable Encryption functionallity" OFF)
option(SERIO_BUILD_SHARED "Build Serio as a Shared Library" OFF)

if(NOT SERIO_BUILD_SHARED AND SERIO_ENABLE_ENCRYPTION)
    message(FATAL_ERROR "Static library can't be built when encryption is enabled")
endif()

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serio/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/serio/config.h")
file(GLOB HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serio/*.h"
    "${CMAKE_CURRENT_BINARY_DIR}/gen/serio/config.h")
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/serio/serio.cpp")

set(ENCRYPTION_MACRO "")
set(ENCRYPTION_LIBRARY "")

if(SERIO_ENABLE_ENCRYPTION)
    set(SODIUM_DISABLE_TESTS OFF CACHE BOOL "")
    set(SODIUM_MINIMAL ON CACHE BOOL "")

    add_subdirectory(3rd/libsodium-cmake EXCLUDE_FROM_ALL)
    set(ENCRYPTION_MACRO SERIO_USE_ENCRYPTION)
    set(ENCRYPTION_LIBRARY sodium)

    set_target_properties(sodium PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

set(COMPRESSION_MACRO "")
set(COMPRESSION_LIBRARY "")
set(COMPRESSION_INCLUDE "")
if (SERIO_ENABLE_COMPRESSION)
    set(ZSTD_BUILD_STATIC ON CACHE BOOL "")
    set(ZSTD_BUILD_TESTS OFF CACHE BOOL "")
    set(ZSTD_BUILD_SHARED OFF CACHE BOOL "")
    set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "")

    add_subdirectory(3rd/zstd/build/cmake EXCLUDE_FROM_ALL)
    set(COMPRESSION_LIBRARY libzstd_static)
    set(COMPRESSION_MACRO SERIO_USE_COMPRESSION)
    set(COMPRESSION_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/3rd/zstd/lib")
endif()

if (SERIO_ENABLE_COMPRESSION OR SERIO_ENABLE_ENCRYPTION)
    if(SERIO_BUILD_SHARED)
        add_library(serio SHARED ${HEADERS} ${SOURCES})
    else()
        add_library(serio STATIC ${HEADERS} ${SOURCES})
    endif()

    target_include_directories(serio PRIVATE
        "${COMPRESSION_INCLUDE}")
    target_link_libraries(serio PRIVATE
        ${COMPRESSION_LIBRARY} ${ENCRYPTION_LIBRARY})
    target_compile_definitions(serio PUBLIC
        ${COMPRESSION_MACRO} ${ENCRYPTION_MACRO})
    set(INCLUDE_TYPE PUBLIC)
else()
    add_library(serio INTERFACE)
    set(INCLUDE_TYPE INTERFACE)
endif()

target_include_directories(serio ${INCLUDE_TYPE}
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rd/rapidxml>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rd/rapidjson/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/gen/>"
    "$<INSTALL_INTERFACE:include/>"
    "$<INSTALL_INTERFACE:include/serio/3rd>")

if(SERIO_INSTALL_LIB)
    install(TARGETS serio
        EXPORT serioConfig
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
    install(FILES ${HEADERS}
        DESTINATION include/serio)
    install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rapidxml/
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/rapidjson/include/
        DESTINATION include/serio/3rd)
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

    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(INSTALL_GTEST OFF CACHE BOOL "")
    add_subdirectory(./3rd/googletest/)

    enable_testing()
    include(GoogleTest)

    file(GLOB SERIO_STRESS_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/stress/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/stress/*.cpp")
    add_executable(serio_stress_tests ${HEADERS} ${SERIO_STRESS_SOURCES})
    target_link_libraries(serio_stress_tests PUBLIC serio gtest gtest_main)
    gtest_discover_tests(serio_stress_tests)

    file(GLOB CEREAL_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/cereal/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/cereal/*.cpp")
    add_executable(cereal_tests ${CEREAL_TEST_SOURCES})
    target_include_directories(cereal_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/3rd/doctest/")
    target_link_libraries(cereal_tests PUBLIC serio gtest gtest_main)
    add_test(NAME cereal_tests COMMAND cereal_tests)

    find_package(Boost CONFIG QUIET)
    if (Boost_FOUND)
        message(STATUS "Boost found - enabling Boost serialization tests")
        target_link_libraries(serio_stress_tests PUBLIC Boost::headers)
        target_compile_definitions(serio_stress_tests PUBLIC SERIO_TEST_BOOST BOOST_ALL_NO_LIB)
    else()
        message(STATUS "Boost not found - skipping Boost serialization tests")
    endif()

    find_package(Qt6 COMPONENTS Core QUIET)
    if (Qt6_FOUND)
        message(STATUS "Qt6 found - enabling Qt serialization tests")
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
        target_link_libraries(serio_stress_tests PUBLIC Qt6::Core)
        target_compile_definitions(serio_stress_tests PUBLIC SERIO_TEST_QT)
    else()
        message(STATUS "Qt6 not found - skipping Qt serialization tests")
    endif()

    file(GLOB SERIO_TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/serio/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/serio/*.cpp")
    add_executable(serio_tests ${HEADERS} ${SERIO_TEST_SOURCES})
    target_link_libraries(serio_tests PUBLIC serio gtest gtest_main)
    target_compile_features(serio_tests PRIVATE cxx_std_23)
    gtest_discover_tests(serio_tests)

    if (Boost_FOUND)
        target_link_libraries(serio_tests PUBLIC Boost::headers)
        target_compile_definitions(serio_tests PUBLIC SERIO_TEST_BOOST BOOST_ALL_NO_LIB)
    endif()
    if (Qt6_FOUND)
        target_link_libraries(serio_tests PUBLIC Qt6::Core)
        target_compile_definitions(serio_tests PUBLIC SERIO_TEST_QT)
    endif()
endif()
