cmake_minimum_required(VERSION 3.10.2)
project(HSE_2020_torrent_project)

set(Boost_USE_MULTITHREADED ON)
set(CMAKE_CXX_STANDARD 17)

add_executable(client main.cpp source/info_helper.cpp include/info_helper.h source/API.cpp include/API.h source/client.cpp include/client.h)

target_link_libraries(client torrent-rasterbar boost_system)