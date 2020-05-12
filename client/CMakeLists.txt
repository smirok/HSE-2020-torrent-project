cmake_minimum_required(VERSION 3.10.2)
project(HSE_2020_torrent_project)

find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

enable_testing()

set(Boost_USE_MULTITHREADED ON)
set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

include_directories(${GTEST_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(Test test/run_tests.cpp test/test.cpp

        source/InfoHelper.cpp include/InfoHelper.h source/API.cpp include/API.h
        source/View.cpp include/View.h)

target_link_libraries(Test torrent-rasterbar boost_system gtest)
