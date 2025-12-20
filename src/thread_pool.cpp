#include "headers/thread_pool.h"
#include <iostream>

thread_pool::thread_pool(task_queue& queue, size_t num_threads) : target_queue(queue) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this, i] {
            while (true) {
                int client_fd = target_queue.pop();

                if (client_fd == -1) {
                    break;
                }

                // std::cout << "[Worker " << i << "] Handling connection on FD: "
                //           << client_fd << std::endl;

                handle_client(client_fd);
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