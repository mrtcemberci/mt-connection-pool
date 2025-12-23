#include "headers/thread_pool.h"
#include <iostream>

thread_pool::thread_pool(task_queue& queue, size_t num_threads) : target_queue(queue) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this, i] {
            while (true) {
                Task task = target_queue.pop();

                if (task.client_fd == -1) {
                    break;
                }

                handle_client(task);
            }
        });
    }
}

thread_pool::~thread_pool() {
    target_queue.shutdown();

    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}