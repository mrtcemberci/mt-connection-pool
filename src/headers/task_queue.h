#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

class task_queue {
public:
    task_queue();
    ~task_queue() = default;

    void push(int client_fd);

    int pop();

    void shutdown();

private:
    std::queue<int> internal_queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop_flag;
};

#endif