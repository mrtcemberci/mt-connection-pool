#include "headers/worker.h"
#include <iostream>
#include <string>
#include <winsock2.h>

bool handle_client(const Task& task, task_queue& q) {
    if (task.type == TaskType::READ_READY) {
        char buffer[4096];
        int bytes_received = recv(task.client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) return false;
            closesocket(task.client_fd);
            return true;
        }
        if (bytes_received == 0) {
            closesocket(task.client_fd);
            return true;
        }

        buffer[bytes_received] = '\0';
        std::string request(buffer);

        static const std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "Connection: keep-alive\r\n"
                    "Content-Length: 12\r\n"
                    "\r\n"
                    "Hello World!";

        int bytes_sent = send(task.client_fd, response.c_str(), (int)response.length(), 0);

        if (bytes_sent < 0) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                return false;
            }
            closesocket(task.client_fd);
            return true;
        }

        if (bytes_received > 20 && memcmp(buffer, "Connection: close", 17) == 0) {
            closesocket(task.client_fd);
            return true;
        }

        return false;
    }
    return false;
}