#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>
#include <thread>

#include "task_queue.h"
#include "headers/task_queue.h"

class thread_pool {
public:
    thread_pool(task_queue& queue, size_t num_threads);

    ~thread_pool();

    // prevents copying of the thread pool
    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

private:
    std::vector<std::thread> workers;
    task_queue& target_queue;
};

#endif