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
#include <beehive/worker.h>
#include <memory>
#include <vector>
#include <stack>
#include <queue>
#include <mutex>
#include <future>
#include <unordered_map>

namespace beehive {
class Pool {
    public:
        using Callable = std::function<void()>;

        class SchedulableTask {
            public:
                SchedulableTask(Callable);

                std::shared_future<void>& future();
                Callable& callable();
                std::promise<void>& promise();

            private:
                Callable mCallable;
                std::promise<void> mPromise;
                std::shared_future<void> mFuture;
        };

        Pool(size_t = 0);
        ~Pool();

        size_t size() const;
        std::shared_future<void> schedule(Callable);

        bool idle() const;
        std::shared_ptr<SchedulableTask> task();

        std::unordered_map<std::thread::id, Worker::Stats> stats();

    private:
        Worker* at(size_t) const;

        std::vector<std::unique_ptr<Worker>> mWorkers;

        mutable std::mutex mTasksMutex;
        std::queue<std::shared_ptr<SchedulableTask>> mTasks;
};
}
