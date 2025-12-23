#include "headers/worker.h"
#include <iostream>
#include <string>
#include <winsock2.h>

void handle_client(Task task, task_queue& q) {
    if (!task.request_data.empty()) {
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello World!";
        task.request_data = response;
        q.push_completed(task);
    }
}