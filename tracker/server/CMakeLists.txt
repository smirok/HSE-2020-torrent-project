cmake_minimum_required(VERSION 3.15)
project(server)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)
add_executable(client
        src/client.cpp)

target_link_libraries(client boost_system boost_thread pthread)


include_directories(include)
add_executable(server
        src/main.cpp
        src/server.cpp
        include/server.hpp)
target_link_libraries(server boost_system boost_thread pthread)