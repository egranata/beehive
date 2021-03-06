/*
Copyright 2020 Google LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <stddef.h>
#include <beehive/task.h>
#include <beehive/pq.h>
#include <beehive/idempotency.h>
#include <beehive/worker.h>
#include <memory>
#include <vector>
#include <stack>
#include <queue>
#include <mutex>

namespace beehive {
class Pool {
    public:
        Pool(size_t = 0);
        ~Pool();

        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;
        Pool(const Pool&&) = delete;
        Pool& operator=(const Pool&&) = delete;

        size_t size() const;
        std::shared_future<void> schedule(Task::Callable, Task::Priority = Task::DefaultPriority);

        bool idle() const;
        std::shared_ptr<Task> task();

        std::vector<Worker::Stats> stats();
        Worker::View worker(int);

        void dump();

        void addworker();

        IdempotencySet& idempotency();

    private:
        Worker* at(size_t) const;

        void foreachworker(std::function<void(std::unique_ptr<Worker>&)>);

        mutable std::recursive_mutex mWorkersMutex;
        std::vector<std::unique_ptr<Worker>> mWorkers;

        PriorityQueue<Task::Priority, std::shared_ptr<Task>> mTasks;

        IdempotencySet mIdempotencySet;
};
}
