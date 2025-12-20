#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

void latency_test(int target_rps, int duration_sec) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return;
    }

    std::vector<double> latencies;
    auto start_test = std::chrono::high_resolution_clock::now();
    auto interval = std::chrono::microseconds(1000000 / target_rps);

    std::cout << "Starting Independent Latency Test..." << std::endl;

    for (int i = 0; i < target_rps * duration_sec; ++i) {
        auto intended_start = start_test + (i * interval);

        while (std::chrono::high_resolution_clock::now() < intended_start);

        SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8080);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(s, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            send(s, "GET / HTTP/1.1\r\n\r\n", 18, 0);
            char buf[512];
            recv(s, buf, 512, 0);
        }
        closesocket(s);

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> lat = end - intended_start;
        latencies.push_back(lat.count());
    }

    std::sort(latencies.begin(), latencies.end());
    std::cout << "\n--- Latency Results ---" << std::endl;
    std::cout << "P50 Latency: " << latencies[static_cast<size_t>(latencies.size() * 0.5)] << " ms" << std::endl;
    std::cout << "P99 Latency: " << latencies[static_cast<size_t>(latencies.size() * 0.99)] << " ms" << std::endl;

    WSACleanup();
}

int main() {
    // Run independently at 50 Requests Per Second for 5 seconds
    latency_test(50, 5);
    return 0;
}