# mt-connection-pool

## Introduction

A high-performance Multi-Reactor (One-Loop-Per-Thread) TCP server
built on Windows. This project was built to help me learn C++ systems principles
and improve my systems skills across Windows.

## The Architecture 
 This uses a Master-Worker model with a worker pool.

### Master/Main Thread
Sits in a while true loop only running accept for new connections and adding them to the shared work queue.

### Worker Reactors
Set to 4 threads (Can be changed in server.cpp) with their own private poller. This means zero lock contention once 
a client is assigned to a thread.

## Key Features
### Data inspection
The workers do not inspect the received data, right now it just sends a generic reply. This can be changed by just
altering worker.cpp as the code is strongly decoupled.

### TCP_NODELAY 
Nagle's algorithm is disabled because Windows intentionally buffers data to see if more data comes through.

### Persistent Benchmark
Both benchmark tools maintain the TCP connection over the several requests to reduce the TCP handshake overhead.

## The Numbers

### Latency
On my latest tests (the commits up to this readme) I have achieved
**P50** and **P99** score of **0.22ms** and **0.70ms** on average respectively.
These numbers were adjusted for factors such as coordinated ommision by introducing a delay and having a target RPS.
### Throughput
Following the same structure as the latency tests, I achieved an average throughput of **70k RPS+**. This is much higher than my original results
of roughly **2k RPS+** due to adding persistent connections.

## Windows TAX
WSAPoll is not good compared to Epoll on LINUX which uses Edge-triggering. WSAPoll is a slow O(n) iteration each time.

## Build
`mkdir build && cd build`

`cmake ..`

`cmake --build .`

## Clean
In **build/** run `cmake --build . --target clean`