cmake_minimum_required(VERSION 2.8)
project(Serio)
set(CMAKE_CXX_STANDARD 11)

include_directories(.)
add_library(${PROJECT_NAME} INTERFACE)
install(FILES "serio.h" DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    find_package(Threads REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(Tests "tests.cpp")
    target_link_libraries(Tests ${PROJECT_NAME})
    target_link_libraries(Tests ${GTEST_LIBRARIES})
    target_link_libraries(Tests ${CMAKE_THREAD_LIBS_INIT})
endif(BUILD_TESTS)
