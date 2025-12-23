cmake_minimum_required(VERSION 3.10)

project(mt_connection_pool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(mt_connection_pool
        src/server.cpp
        src/task_queue.cpp
        src/thread_pool.cpp
        src/worker.cpp
        src/poller.cpp
)

add_executable(latency_bench src/latency_benchmark.cpp)
add_executable(throughput_bench src/throughput_benchmark.cpp)

target_link_libraries(mt_connection_pool PRIVATE Threads::Threads ws2_32)
target_link_libraries(latency_bench PRIVATE ws2_32)
target_link_libraries(throughput_bench PRIVATE ws2_32)