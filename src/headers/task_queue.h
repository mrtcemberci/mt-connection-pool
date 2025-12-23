#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

struct Task {
    int client_fd;
    std::string request_data;
};

class task_queue {
public:
    task_queue();
    ~task_queue() = default;

    void push(Task task);

    Task pop();

    void push_completed(Task task);

    bool try_pop_completed(Task& task);

    void shutdown();

private:
    std::queue<Task> internal_queue;
    std::queue<Task> completed_queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop_flag;
};

#endif