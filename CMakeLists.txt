cmake_minimum_required(VERSION 2.8)
project(Serio LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)
add_library(${PROJECT_NAME} INTERFACE)
install(FILES "serio.h" DESTINATION ${CMAKE_INSTALL_PREFIX}/include/serio/)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    if(WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif(WIN32)

    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(INSTALL_GTEST OFF CACHE BOOL "")
    add_subdirectory(./googletest/)

    add_executable(Tests "serio.h" "tests.cpp")
    target_link_libraries(Tests ${PROJECT_NAME} gtest_main)
    add_test(NAME SerioTest COMMAND Tests)
endif(BUILD_TESTS)
