cmake_minimum_required(VERSION 3.15)
project(examp_boost_asio_udp_echo)

set(CMAKE_CXX_STANDARD 17)


add_executable(client client.cpp)
target_link_libraries(client boost_system boost_thread pthread)
add_executable(server server.cpp)
target_link_libraries(server boost_system boost_thread pthread)