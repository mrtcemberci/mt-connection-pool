#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <vector>
#include <map>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "headers/task_queue.h"
#include "headers/thread_pool.h"
#include "headers/poller.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"

void set_non_blocking(SOCKET s) {
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);
}

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    task_queue q;
    thread_pool pool(q, 4); // 4 Worker Threads
    Poller poller;          // The Event Manager

    struct addrinfo *result = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

    SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    set_non_blocking(listen_socket);
    poller.add(listen_socket, EventType::READ);

    std::cout << "Server (Async Reactor) started on port " << DEFAULT_PORT << "..." << std::endl;

    // Buffer to hold responses waiting to be sent
    std::map<int, std::string> response_buffers;

    while (true) {
        Task completed_task;
        // This processes the completed tasks
        while (q.try_pop_completed(completed_task)) {
            response_buffers[completed_task.client_fd] = completed_task.request_data;
            poller.add(completed_task.client_fd, EventType::WRITE);
        }

        std::vector<IOEvent> events = poller.wait(100); // 100ms timeout

        for (const auto& event : events) {

            if (event.fd == listen_socket) {
                SOCKET client = accept(listen_socket, NULL, NULL);
                if (client != INVALID_SOCKET) {
                    set_non_blocking(client);
                    poller.add(client, EventType::READ);
                    // std::cout << "New connection: " << client << std::endl;
                }
            }

            else if (event.type == EventType::READ) {
                char buffer[1024];
                int bytes = recv(event.fd, buffer, sizeof(buffer) - 1, 0);

                if (bytes > 0) {
                    buffer[bytes] = '\0';

                    poller.remove(event.fd);

                    Task t;
                    t.client_fd = static_cast<int>(event.fd);
                    t.request_data = std::string(buffer);

                    q.push(t);
                } else {
                    closesocket(event.fd);
                    poller.remove(event.fd);
                    response_buffers.erase(static_cast<int>(event.fd));
                }
            }

            else if (event.type == EventType::WRITE) {
                int fd = static_cast<int>(event.fd);

                if (response_buffers.count(fd)) {
                    const std::string& data = response_buffers[fd];
                    send(fd, data.c_str(), static_cast<int>(data.size()), 0);

                    response_buffers.erase(fd);
                    poller.remove(fd);
                    closesocket(fd);
                }
            }
        }
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}