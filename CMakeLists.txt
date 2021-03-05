cmake_minimum_required(VERSION 3.17)
project(Multithreading)

set(CMAKE_CXX_STANDARD 17)

# For tests
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -O2 -g")

# For benchmarks
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_subdirectory(benchmark)

include_directories(benchmark/googletest/googletest/include/gtest)

add_executable(
        FindIfTests

        benchmark/googletest/googletest/src/gtest_main.cc
        find_if/find_if_tests.cpp

        utilities.cpp
        find_if/find_if.cpp
)
target_link_libraries(FindIfTests gtest)

add_executable(
        FindIfBench

        find_if/find_if_bench.cpp

        utilities.cpp
        find_if/find_if.cpp
)
target_link_libraries(FindIfBench benchmark::benchmark)

add_executable(
        HashCollisionTests

        benchmark/googletest/googletest/src/gtest_main.cc
        hash_collision/hash_tests.cpp

        hash_collision/hash.cpp
        hash_collision/hash_map.cpp
        hash_collision/hash_string.cpp
        hash_collision/hash_collision_searcher.cpp
        utilities.cpp
)
target_link_libraries(HashCollisionTests gtest)

add_executable(
        HashCollisionBench

        hash_collision/hash_bench.cpp

        hash_collision/hash.cpp
        hash_collision/hash_map.cpp
        hash_collision/hash_string.cpp
        hash_collision/hash_collision_searcher.cpp
        utilities.cpp
)
target_link_libraries(HashCollisionBench benchmark::benchmark)

add_executable(
        UnkeptPromisesTests

        benchmark/googletest/googletest/src/gtest_main.cc
        unkept_promises/promise_tests.cpp
)
target_link_libraries(UnkeptPromisesTests gtest)
