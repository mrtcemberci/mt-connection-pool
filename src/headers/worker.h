#ifndef WORKER_HPP
#define WORKER_HPP

#include "task_queue.h"

bool handle_client(const Task& task, task_queue& queue);

#endif