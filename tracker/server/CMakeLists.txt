cmake_minimum_required(VERSION 3.10) #upgrade to 3.15
project(server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wshadow -pedantic")

find_package(Boost 1.71.0 REQUIRED)
find_package(Boost 1.71.0 REQUIRED COMPONENTS system program_options)
find_package(Threads REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(server
        src/main.cpp
        src/UDP_server.cpp
        include/UDP_server.hpp
        src/TorrentDataBase.cpp
        include/TorrentDataBase.hpp
        )
target_link_libraries(server
        pthread
        Boost::program_options)