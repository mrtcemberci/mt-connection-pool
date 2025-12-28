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

constexpr int MAX_THREADS = 128;

class lock_free_queue : public task_queue {
public:

    struct Node;

    using TaggedPtr = uintptr_t;

    struct Node {
        // The data the node holds (task)
        Task data;
        // a tagged ptr to the next task for the linked queue
        // the tagged data contains a tag (timestamp-ish) and the data.
        // Used to prevent A-B-A problem
        std::atomic<TaggedPtr> next;

        Node(Task t) : data(t), next(0) {}
        Node() : data({-1, TaskType::READ_READY, ""}), next(0) {}
    };
private:

    // Self explanatory 
    std::atomic<TaggedPtr> head;
    std::atomic<TaggedPtr> tail;
    std::atomic<bool> stop_flag;

    // a struct used in an array of hazard pointers.
    // This used to prevent use after frees
    struct HP_Record {
        std::atomic<Node*> pointer;
        char padding[64 - sizeof(std::atomic<Node*>)];
    };
    static HP_Record hazard_pointers[MAX_THREADS];

    // deletes a node safely.
    void retire_node(Node* node);
    // Attempts to delete a list of nodes safely, skipping nodes in hazard state
    void scan_hazard_pointers(std::vector<Node*>& retired_list);

    Node* get_ptr(TaggedPtr tp) const {
        return reinterpret_cast<Node*>(tp & 0x0000FFFFFFFFFFFFULL);
    }

    uint16_t get_tag(TaggedPtr tp) const {
        return static_cast<uint16_t>(tp >> 48);
    }

    TaggedPtr make_tagged(Node* p, uint16_t tag) const {
        return reinterpret_cast<TaggedPtr>(p) | (static_cast<TaggedPtr>(tag) << 48);
    }

public:
    lock_free_queue();
    virtual ~lock_free_queue();

    void push(Task task) override;
    Task try_pop_new() override;
    bool is_shutdown() override;
    void shutdown() override;
};

#endif