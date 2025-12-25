#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <algorithm>
#include <iostream>
#include "headers/poller.h"

Poller::Poller() {}

void Poller::add(SOCKET fd, EventType type) {
    WSAPOLLFD pfd;
    pfd.fd = fd;
    pfd.events = (type == EventType::READ) ? POLLRDNORM : POLLWRNORM;
    pfd.revents = 0;
    poll_fds.push_back(pfd);
}

void Poller::remove(SOCKET fd) {
    auto it = std::remove_if(poll_fds.begin(), poll_fds.end(), [fd](const WSAPOLLFD& pfd) {
        return pfd.fd == fd;
    });
    poll_fds.erase(it, poll_fds.end());
}

std::vector<IOEvent> Poller::wait(int timeout_ms) {
    std::vector<IOEvent> triggered_events;

    if (poll_fds.empty()) {
        return triggered_events;
    }

    // WSAPoll returns the number of structures with non-zero revents
    int ret = WSAPoll(poll_fds.data(), static_cast<ULONG>(poll_fds.size()), timeout_ms);

    if (ret > 0) {
        for (const auto& pfd : poll_fds) {
            if (pfd.revents & (POLLRDNORM | POLLHUP | POLLERR)) {
                triggered_events.push_back({pfd.fd, EventType::READ});
            }
        }
    } else if (ret == SOCKET_ERROR) {
        std::cerr << "WSAPoll failed: " << WSAGetLastError() << std::endl;
    }

    return triggered_events;
}