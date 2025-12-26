#include "headers/task_queue.h"

task_queue_lock::task_queue_lock() : stop_flag(false) {}

void task_queue_lock::push(Task task) {
    std::lock_guard lock(mtx);
    internal_queue.push(task);
}

Task task_queue_lock::try_pop_new() {
    std::lock_guard<std::mutex> lock(mtx);
    if (internal_queue.empty()) {
        return {-1, TaskType::READ_READY,""};
    }
    Task t = internal_queue.front();
    internal_queue.pop();
    return t;
}

bool task_queue_lock::is_shutdown() {
    std::lock_guard lock(mtx);
    return stop_flag;
}

void task_queue_lock::shutdown() {
    std::lock_guard lock(mtx);
    stop_flag = true;
}