#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "headers/task_queue.h"
#include "headers/thread_pool.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8080"

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    task_queue q;
    thread_pool pool(q, 4);
    std::cout << "Server started. Listening on port " << DEFAULT_PORT << "..." << std::endl;

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
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    while (true) {
        SOCKET client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue; 
        }

        //std::cout << "New connection accepted. Pushing FD " << client_socket << " to queue." << std::endl;

        // Event loop
        // We read the data HERE. If this were epoll, we would only be here
        // if we have an IO read event
        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';

            Task new_task;
            new_task.client_fd = static_cast<int>(client_socket);
            new_task.request_data = std::string(buffer);

            q.push(new_task);
        } else {
            closesocket(client_socket);
        }
    }

    closesocket(listen_socket);
    WSACleanup();

    return 0;
}