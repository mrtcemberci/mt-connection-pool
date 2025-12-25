#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

std::atomic<int> total_completed(0);
std::atomic<bool> running(true);

void throughput_worker(const char* ip, int port) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(s, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) != 0) {
        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
        closesocket(s);
        return;
    }

    while (running) {
        const char* request = "GET / HTTP/1.1\r\n\r\n";
        if (send(s, request, 18, 0) == SOCKET_ERROR) {
            break;
        }

        char buf[1024];
        int bytes = recv(s, buf, sizeof(buf), 0);

        if (bytes > 0) {
            total_completed++;
        } else {
            break;
        }
    }

    closesocket(s);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    int num_threads = 8;
    std::vector<std::thread> workers;

    std::cout << "Starting throughput test for 10 seconds..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    for(int i = 0; i < num_threads; ++i)
        workers.emplace_back(throughput_worker, "127.0.0.1", 8080);

    std::this_thread::sleep_for(std::chrono::seconds(10));
    running = false;

    for(auto& t : workers) t.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Throughput: " << total_completed / duration.count() << " req/sec" << std::endl;
    WSACleanup();
    return 0;
}