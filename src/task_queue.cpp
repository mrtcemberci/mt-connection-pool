#include "headers/task_queue.h"

task_queue::task_queue() : stop_flag(false) {}

void task_queue::push(Task task) {
    std::lock_guard lock(mtx);
    internal_queue.push(task);
    cv.notify_one();
}

Task task_queue::pop() {
    std::unique_lock<std::mutex> lock(mtx);

    cv.wait(lock, [this]() {
        return !internal_queue.empty() || stop_flag;
    });

    if (stop_flag && internal_queue.empty()) {
        return {-1, ""};
    }

    Task task = internal_queue.front();
    internal_queue.pop();
    return task;
}

void task_queue::shutdown() {
    std::lock_guard lock(mtx);
    stop_flag = true;
    cv.notify_all();
}