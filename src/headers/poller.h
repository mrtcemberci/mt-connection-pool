#ifndef POLLER_HPP
#define POLLER_HPP

#include <vector>
#include <winsock2.h>

enum class EventType {
    READ,
};

struct IOEvent {
    SOCKET fd;
    EventType type;
};

class Poller {
public:
    Poller();
    ~Poller() = default;

    // Register a file descriptor to monitor for specific events
    void add(SOCKET fd, EventType type);

    // Stop monitoring a file descriptor
    void remove(SOCKET fd);

    // Wait for events (timeout in milliseconds). Returns list of triggered events.
    std::vector<IOEvent> wait(int timeout_ms = -1);

private:
    std::vector<WSAPOLLFD> poll_fds;
};

#endif