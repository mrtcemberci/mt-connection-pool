#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

std::atomic<int> total_completed(0);
std::atomic<bool> running(true);

void throughput_worker(const char* ip, int port) {
    while (running) {
        SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr(ip);

        if (connect(s, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == 0) {
            send(s, "GET / HTTP/1.1\r\n\r\n", 18, 0);
            char buf[1024];
            recv(s, buf, 1024, 0);
            total_completed++;
        }
        closesocket(s);
    }
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