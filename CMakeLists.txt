cmake_minimum_required(VERSION 3.10)

project(mt_connection_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(mt_connection_pool src/server.cpp)

target_link_libraries(mt_connection_pool PRIVATE Threads::Threads)