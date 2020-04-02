cmake_minimum_required(VERSION 3.10.2)
project(HSE_2020_torrent_project)

set(Boost_USE_MULTITHREADED ON)
set(CMAKE_CXX_STANDARD 17)
set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(client main.cpp source/InfoHelper.cpp include/InfoHelper.h source/API.cpp include/API.h
        source/View.cpp include/View.h)
add_executable(tmaker source/make_torrent.cpp include/make_torrent.h)

target_link_libraries(client torrent-rasterbar boost_system)
target_link_libraries(tmaker torrent-rasterbar boost_system)