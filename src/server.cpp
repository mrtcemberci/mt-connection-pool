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

    task_queue_lock q;
    thread_pool pool(q, 4); // 4 Worker Threads

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

    std::cout << "Server (Async Reactor) started on port " << DEFAULT_PORT << "..." << std::endl;

    while (true) {
        SOCKET client = accept(listen_socket, NULL, NULL);
        if (client != INVALID_SOCKET) {
            set_non_blocking(client);
            // Disables Nagle algorithm, No TCP_DELAY, Windows does not wait for more data before sending
            int no_delay = 1;
            setsockopt(client, IPPROTO_TCP, TCP_NODELAY, (char*)&no_delay, sizeof(no_delay));

            q.push(Task{static_cast<int>(client), TaskType::NEW_CONNECTION, ""});
        }
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}