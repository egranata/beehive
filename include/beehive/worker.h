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
#include <bits/stdint-uintn.h>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <beehive/mq.h>
#include <chrono>
#include <string>
#include <vector>
#include <beehive/timecounter.h>
#include <beehive/mq.h>

namespace beehive {
class Pool;

class Worker : public SignalingQueue::Handler {
    public:
        struct Stats {
            uint64_t messages;
            uint64_t runs;
            std::chrono::milliseconds idle;
            std::chrono::milliseconds active;

            bool operator==(const Stats&) const;
            bool operator!=(const Stats&) const;
        };

        class View {
            public:
                std::string name();
                void name(const char*);
                int id() const;
                std::thread::id tid();
                std::thread::native_handle_type nativeid();
                Worker::Stats stats();
                std::vector<bool> affinity();
                void affinity(const std::vector<bool>&);

                explicit operator bool() const;

                static View empty();

            private:
                friend class Worker;
                View(Worker*);

                Worker* mWorker;
        };

        Worker(Pool*, int);
        ~Worker();

        void send(Message);

        void exit();
        void task();
        void dump();

        View view();

        std::string name();
        void name(const char*);
        int id() const;

        Stats stats();
        std::thread::id tid();
        std::thread::native_handle_type nativeid();

        std::vector<bool> affinity();
        void affinity(const std::vector<bool>&);

        Worker(Worker&&) = default;

        void onBeforeMessage() override;
        void onAfterMessage() override;

        Result onNop(const Message::NOP_Data&) override;
        Result onExit(const Message::EXIT_Data&) override;
        Result onTask(const Message::TASK_Data&) override;
        Result onDump(const Message::DUMP_Data&) override;
    private:
        friend class Worker::View;

        class AtomicStats {
            public:
                AtomicStats();

                Stats load();
                void message();
                void run();
                TimeCounter& active();
                TimeCounter& idle();

            private:
                std::atomic<uint64_t> mMessages{0};
                std::atomic<uint64_t> mRuns{0};
                TimeCounter mActive;
                TimeCounter mIdle;
        };

        Pool* mParent;
        int mId;

        std::thread mWorkThread;
        SignalingQueue mMsgQueue;
        AtomicStats mStats;

        void WorkLoop();
};
}
