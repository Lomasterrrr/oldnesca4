/*
 *          NESCA4
 *   Сделано от души 2023.
 * Copyright (c) [2023] [lomaster]
 * SPDX-License-Identifier: BSD-3-Clause
 *
*/

#include "../include/nescathread.h"

thread_pool::thread_pool(size_t numThreads) : stop(false)
{
  for (size_t i = 0; i < numThreads; ++i) {
    workers.emplace_back([this] {
      while (true) {
        std::function<void()> task; {
          std::unique_lock<std::mutex> lock(queueMutex);
          condition.wait(lock, [this] { return stop || !tasks.empty(); });
          if (stop && tasks.empty()) {return;}
            task = std::move(tasks.front());
            tasks.pop();
          }
          task();
        }
      }
    );
  }
}

thread_pool::~thread_pool()
{
  {
    std::unique_lock<std::mutex> lock(queueMutex);
    stop = true;
  }

  condition.notify_all();
  for (std::thread& worker : workers){worker.join();}
}
