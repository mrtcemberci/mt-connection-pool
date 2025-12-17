#include "headers/task_queue.h"

task_queue::task_queue() : stop_flag(false) {}

void task_queue::push(int client_fd) {
    std::lock_guard lock(mtx);
    internal_queue.push(client_fd);
    cv.notify_one();
}

int task_queue::pop() {
    std::unique_lock lock(mtx);

    cv.wait(lock, [this]() {
        return !internal_queue.empty() || stop_flag;
    });

    if (stop_flag && internal_queue.empty()) {
        return -1;
    }

    int client_fd = internal_queue.front();
    internal_queue.pop();
    return client_fd;
}

void task_queue::shutdown() {
    std::lock_guard lock(mtx);
    stop_flag = true;
    cv.notify_all();
}