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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <beehive/mq.h>

namespace beehive {
class Pool;

class Worker {
    public:
        struct Stats {
            uint64_t messages;
            uint64_t runs;
        };

        Worker(Pool*);
        ~Worker();

        void send(Message);

        void exit();
        void task();

        Stats stats();
        std::thread::id tid();

        Worker(Worker&&) = default;
    private:
        class AtomicStats {
            public:
                AtomicStats();

                Stats load();
                void message();
                void run();

            private:
                std::atomic<uint64_t> mMessages{0};
                std::atomic<uint64_t> mRuns{0};
        };

        Pool* mParent;

        std::thread mWorkThread;
        SignalingQueue mMsgQueue;
        AtomicStats mStats;

        void WorkLoop();
};
}
