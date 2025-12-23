#include "headers/worker.h"
#include <iostream>
#include <string>
#include <winsock2.h>

void handle_client(Task task) {
    if (!task.request_data.empty()) {
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello World!";

        // Keep this as send for now, later change it to a IO event out
        send(task.client_fd, response.c_str(), static_cast<int>(response.length()), 0);
    }

    closesocket(task.client_fd);
}