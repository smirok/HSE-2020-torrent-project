cmake_minimum_required(VERSION 3.10) #upgrade to 3.15
project(server)

set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(server
        src/main.cpp
        src/server.cpp
        include/server.hpp)
target_link_libraries(server boost_system boost_thread pthread)