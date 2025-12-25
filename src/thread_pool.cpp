#include "headers/thread_pool.h"
#include <iostream>
#include "headers/poller.h"
#include <windows.h>

thread_pool::thread_pool(task_queue& queue, size_t num_threads) : target_queue(queue) {
    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this, i] {
            SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(1) << i);
            Poller private_poller;
            while (true) {
                Task newTask = target_queue.try_pop_new();

                if (newTask.client_fd == -1 && target_queue.is_shutdown()) {
                    break;
                }

                if (newTask.client_fd != -1) {
                    // This should be a new connection always
                    private_poller.add(newTask.client_fd, EventType::READ);
                }

                constexpr int timeout = 0;
                std::vector<IOEvent> events = private_poller.wait(timeout);

                for (const auto& event : events) {
                    if (event.type == EventType::READ) {
                        Task t;
                        t.client_fd = static_cast<int>(event.fd);
                        t.type = TaskType::READ_READY;

                        if (handle_client(t, target_queue)) {
                            private_poller.remove(event.fd);
                        }
                    }
                }
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