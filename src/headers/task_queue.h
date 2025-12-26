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
    virtual ~task_queue() = default;
    virtual void push(Task task) = 0;
    virtual Task try_pop_new() = 0;
    virtual bool is_shutdown() = 0;
    virtual void shutdown() = 0;
};

class task_queue_lock : public task_queue {
public:
    task_queue_lock();
    void push(Task task) override;
    Task try_pop_new() override;
    bool is_shutdown() override;
    void shutdown() override;

private:
    std::queue<Task> internal_queue;
    std::mutex mtx;
    bool stop_flag;
};

#endif