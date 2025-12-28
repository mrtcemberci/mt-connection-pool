#include "headers/task_queue.h"
#include <algorithm>

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

lock_free_queue::HP_Record lock_free_queue::hazard_pointers[MAX_THREADS];
static std::atomic<int> g_thread_count(0);
thread_local int t_thread_id = -1;
thread_local std::vector<lock_free_queue::Node*> t_retired_list;

int get_thread_id() {
    if (t_thread_id == -1) {
        t_thread_id = g_thread_count.fetch_add(1);
    }
    return t_thread_id;
}

lock_free_queue::lock_free_queue() : stop_flag(false) {
    for(int i = 0; i < MAX_THREADS; ++i) {
        hazard_pointers[i].pointer.store(nullptr);
    }

    Node* dummy = new Node();
    TaggedPtr init = make_tagged(dummy, 0);
    head.store(init);
    tail.store(init);
}

lock_free_queue::~lock_free_queue() {
    while (try_pop_new().client_fd != -1);
    
    Node* final_dummy = get_ptr(head.load());
    delete final_dummy;
}

void lock_free_queue::push(Task task) {
    Node* newNode = new Node(task);
    
    while (true) {
        TaggedPtr tail_tagged = tail.load();
        Node* tail_ptr = get_ptr(tail_tagged);
        uint16_t tail_tag = get_tag(tail_tagged);

        TaggedPtr next_tagged = tail_ptr->next.load();
        Node* next_ptr = get_ptr(next_tagged);
        uint16_t next_tag = get_tag(next_tagged);

        if (tail_tagged == tail.load()) {
            if (next_ptr == nullptr) {
                TaggedPtr new_next = make_tagged(newNode, next_tag + 1);
                if (tail_ptr->next.compare_exchange_strong(next_tagged, new_next)) {
                    TaggedPtr new_tail = make_tagged(newNode, tail_tag + 1);
                    tail.compare_exchange_strong(tail_tagged, new_tail);
                    return;
                }
            } else {
                TaggedPtr new_tail = make_tagged(next_ptr, tail_tag + 1);
                tail.compare_exchange_strong(tail_tagged, new_tail);
            }
        }
    }
}

Task lock_free_queue::try_pop_new() {
    int tid = get_thread_id();
    if (tid >= MAX_THREADS) return {-1, TaskType::READ_READY, ""}; 

    while (true) {
        TaggedPtr head_tagged = head.load();
        Node* head_ptr = get_ptr(head_tagged);
        uint16_t head_tag = get_tag(head_tagged);

        hazard_pointers[tid].pointer.store(head_ptr);
        
        if (head_tagged != head.load()) {
            continue; 
        }

        TaggedPtr tail_tagged = tail.load();
        Node* tail_ptr = get_ptr(tail_tagged);

        TaggedPtr next_tagged = head_ptr->next.load();
        Node* next_ptr = get_ptr(next_tagged);

        if (head_ptr == tail_ptr) {
            if (next_ptr == nullptr) {
                hazard_pointers[tid].pointer.store(nullptr); 
                return {-1, TaskType::READ_READY, ""}; 
            }
            TaggedPtr new_tail = make_tagged(next_ptr, get_tag(tail_tagged) + 1);
            tail.compare_exchange_strong(tail_tagged, new_tail);
        } 
        else {
            hazard_pointers[tid].pointer.store(next_ptr);

            if (head_tagged != head.load() || head_ptr->next.load() != next_tagged) {
                continue; 
            }

            if (next_ptr == nullptr) {
                hazard_pointers[tid].pointer.store(nullptr);
                continue; 
            }

            Task result = next_ptr->data;

            TaggedPtr new_head = make_tagged(next_ptr, head_tag + 1);
            if (head.compare_exchange_strong(head_tagged, new_head)) {
                
                hazard_pointers[tid].pointer.store(nullptr);
                retire_node(head_ptr);
                return result;
            }
        }
        
        hazard_pointers[tid].pointer.store(nullptr);
    }
}

void lock_free_queue::retire_node(Node* node) {
    t_retired_list.push_back(node);

    if (t_retired_list.size() > MAX_THREADS * 2) {
        scan_hazard_pointers(t_retired_list);
    }
}

void lock_free_queue::scan_hazard_pointers(std::vector<Node*>& retired_list) {
    std::vector<Node*> active_hps;
    active_hps.reserve(MAX_THREADS);
    
    for (int i = 0; i < MAX_THREADS; ++i) {
        Node* ptr = hazard_pointers[i].pointer.load();
        if (ptr) active_hps.push_back(ptr);
    }

    std::sort(active_hps.begin(), active_hps.end());

    auto it = retired_list.begin();
    while (it != retired_list.end()) {
        Node* candidate = *it;
        
        if (std::binary_search(active_hps.begin(), active_hps.end(), candidate)) {
            ++it;
        } else {
            delete candidate;
            *it = retired_list.back();
            retired_list.pop_back();
        }
    }
}

bool lock_free_queue::is_shutdown() {
    return stop_flag.load();
}

void lock_free_queue::shutdown() {
    stop_flag.store(true);
}