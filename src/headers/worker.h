#ifndef WORKER_HPP
#define WORKER_HPP

#include "task_queue.h"

void handle_client(Task task, task_queue& queue);

#endif