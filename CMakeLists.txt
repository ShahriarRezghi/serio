cmake_minimum_required(VERSION 2.8)
project(Serio)
set(CMAKE_CXX_STANDARD 11)

include_directories(.)
add_library(${PROJECT_NAME} INTERFACE)
install(FILES "serio.h" DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(Tests "tests.cpp")
    target_link_libraries(Tests gtest gtest_main)
    target_link_libraries(Tests ${PROJECT_NAME})
endif(BUILD_TESTS)
