#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

enum class TaskType { NEW_CONNECTION, READ_READY};

struct Task {
    int client_fd;
    TaskType type;
    std::string data;
};

class task_queue {
public:
    task_queue();
    ~task_queue() = default;

    void push(Task task);

    Task try_pop_new();

    bool is_shutdown();

    void shutdown();

private:
    std::queue<Task> internal_queue;
    std::mutex mtx;
    bool stop_flag;
};

#endif