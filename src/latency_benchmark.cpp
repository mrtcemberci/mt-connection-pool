#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

void latency_test(int target_rps, int duration_sec) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    std::vector<double> latencies;

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::cerr << "Initial connect failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    auto start_test = std::chrono::high_resolution_clock::now();
    auto interval = std::chrono::microseconds(1000000 / target_rps);

    std::cout << "Starting Persistent Latency Test..." << std::endl;

    for (int i = 0; i < target_rps * duration_sec; ++i) {
        auto intended_start = start_test + (i * interval);

        while (std::chrono::high_resolution_clock::now() < intended_start);

        const char* req = "GET / HTTP/1.1\r\n\r\n";
        send(s, req, 18, 0);

        char buf[512];
        int r = recv(s, buf, 512, 0);

        auto end = std::chrono::high_resolution_clock::now();

        if (r > 0) {
            std::chrono::duration<double, std::milli> lat = end - intended_start;
            latencies.push_back(lat.count());
        } else {
            std::cerr << "Connection lost during test." << std::endl;
            break;
        }
    }

    closesocket(s);

    std::sort(latencies.begin(), latencies.end());
    if (!latencies.empty()) {
        std::cout << "\n--- Latency Results (Persistent) ---" << std::endl;
        std::cout << "P50 Latency: " << latencies[static_cast<size_t>(latencies.size() * 0.5)] << " ms" << std::endl;
        std::cout << "P99 Latency: " << latencies[static_cast<size_t>(latencies.size() * 0.99)] << " ms" << std::endl;
    }

    WSACleanup();
}
int main() {
    // Run independently at 50 Requests Per Second for 5 seconds
    latency_test(50, 5);
    return 0;
}