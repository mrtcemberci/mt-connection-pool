#include "headers/worker.h"
#include <iostream>
#include <string>

#include <winsock2.h>

void handle_client(int client_fd) {
    char buffer[1024];

    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        //std::cout << "[Worker] Received: " << buffer << std::endl;

        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello World!";
        send(client_fd, response.c_str(), static_cast<int>(response.length()), 0);
    }

    closesocket(client_fd);
    // std::cout << "[Worker] Connection closed on FD: " << client_fd << std::endl;
}