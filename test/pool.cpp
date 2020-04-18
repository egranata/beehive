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

#include <beehive/pool.h>
#include "gtest/gtest.h"
#include <future>
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <algorithm>
#include <beehive/task.h>

using namespace beehive;
using namespace std::chrono_literals;

TEST(Pool, Size) {
    Pool pool(10);
    ASSERT_EQ(10, pool.size());
}

TEST(Pool, Idle) {
    Pool pool(1);
    ASSERT_TRUE(pool.idle());
}

TEST(Pool, OneTask) {
    Pool pool(1);
    int n = 0;
    auto future = pool.schedule([&n] () -> void {
        n = 1;
    });
    future.wait();
    ASSERT_EQ(1, n);
}

TEST(Pool, TwoTasks) {
    Pool pool(1);
    int n = 0;
    int m = 1;
    auto f1 = pool.schedule([&n] () -> void {
        n = 1;
    });
    auto f2 = pool.schedule([&m] () -> void {
        m = 3;
    });
    f2.wait();
    f1.wait();
    ASSERT_EQ(1, n);
    ASSERT_EQ(3, m);
}

TEST(Pool, TwoParallelTasks) {
    Pool pool(2);
    int n = 0;
    int m = 1;
    auto f1 = pool.schedule([&n] () -> void {
        n = 1;
    });
    auto f2 = pool.schedule([&m] () -> void {
        m = 3;
    });
    f2.wait();
    f1.wait();
    ASSERT_EQ(1, n);
    ASSERT_EQ(3, m);
}

TEST(Pool, TwoLongTasks) {
    Pool pool(2);
    int n = 0;
    int m = 1;
    auto f1 = pool.schedule([&n] () -> void {
        n = 1;
        std::this_thread::sleep_for(1s);
    });
    auto f2 = pool.schedule([&m] () -> void {
        std::this_thread::sleep_for(500ms);
        m = 3;
    });
    f2.wait();
    f1.wait();
    ASSERT_EQ(1, n);
    ASSERT_EQ(3, m);
}

TEST(Pool, OneTaskRunsOnce) {
    Pool pool(2);
    bool ran = false;
    auto f1 = pool.schedule([&ran] () -> void {
        if (ran) abort();
        ran = true;
    });
    f1.wait();
    ASSERT_EQ(true, ran);
}

TEST(Pool, CounterStats) {
    Pool pool(3);
    auto f1 = pool.schedule([] ()->void {});
    auto f2 = pool.schedule([] ()->void {});
    auto f3 = pool.schedule([] ()->void {});
    auto f4 = pool.schedule([] ()->void {});
    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();
    auto stats = pool.stats();
    uint64_t messages = 0;
    uint64_t tasks = 0;
    std::for_each(stats.begin(), stats.end(), [&messages, &tasks] (const auto& kv) -> void {
        messages += kv.messages;
        tasks += kv.runs;
    });
    ASSERT_EQ(4, tasks);
    ASSERT_TRUE(messages >= 4);
}

TEST(Pool, TimeStats) {
    Pool pool(3);
    auto f1 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(400ms);
    });
    auto f2 = pool.schedule([] ()->void {});
    auto f3 = pool.schedule([] ()->void {});
    auto f4 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(200ms);
    });
    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();
    auto stats = pool.stats();
    std::chrono::milliseconds active;
    std::for_each(stats.begin(), stats.end(), [&active] (const auto& kv) -> void {
        active += kv.active;
    });
    ASSERT_TRUE(active >= 400ms);
}

TEST(Pool, HyveOfZero) {
    Pool pool;
    ASSERT_TRUE(pool.size() > 0);
}

TEST(Pool, Dump) {
    Pool pool(3);
    std::this_thread::sleep_for(300ms);
    auto f1 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(100ms);
    });
    auto f2 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(25ms);
    });
    auto f3 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(150ms);
    });
    auto f4 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(250ms);
    });
    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();
    pool.dump();
}

TEST(Pool, WorkerViewId) {
    Pool pool(3);
    ASSERT_EQ(0, pool.worker(0).id());
    ASSERT_EQ(1, pool.worker(1).id());
    ASSERT_EQ(2, pool.worker(2).id());

    ASSERT_FALSE(pool.worker(10));
}

TEST(Pool, WorkerName) {
    Pool pool(3);
    pool.worker(1).name("test_worker");

    ASSERT_EQ("test_worker", pool.worker(1).name());
    ASSERT_EQ("worker[0]", pool.worker(0).name());
    ASSERT_EQ("worker[2]", pool.worker(2).name());
}

TEST(Pool, WorkerStats) {
    Pool pool(3);
    std::this_thread::sleep_for(300ms);
    auto f1 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(100ms);
    });
    auto f2 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(25ms);
    });
    auto f3 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(150ms);
    });
    auto f4 = pool.schedule([] ()->void {
        std::this_thread::sleep_for(250ms);
    });
    f1.wait();
    f2.wait();
    f3.wait();
    f4.wait();

    auto stats = pool.stats();
    auto w0 = pool.worker(0).stats();
    auto w1 = pool.worker(1).stats();
    auto w2 = pool.worker(2).stats();

    ASSERT_TRUE(stats.at(0).active <= w0.active);
    ASSERT_TRUE(stats.at(0).idle <= w0.idle);

    ASSERT_TRUE(stats.at(1).active <= w1.active);
    ASSERT_TRUE(stats.at(1).idle <= w1.idle);

    ASSERT_TRUE(stats.at(2).active <= w2.active);
    ASSERT_TRUE(stats.at(2).idle <= w2.idle);
}

TEST(Pool, AddWorker) {
    Pool pool(1);

    ASSERT_EQ(1, pool.size());

    pool.addworker();
    ASSERT_EQ(2, pool.size());

    pool.addworker();
    ASSERT_EQ(3, pool.size());

    ASSERT_EQ("worker[0]", pool.worker(0).name());
    ASSERT_EQ("worker[1]", pool.worker(1).name());
    ASSERT_EQ("worker[2]", pool.worker(2).name());
}

TEST(Pool, TaskPriority) {
    using Instant = std::chrono::time_point<std::chrono::steady_clock>;
    using Delay = std::chrono::milliseconds;
    auto now = [] () -> Instant {
        return std::chrono::steady_clock::now();
    };

    Instant t1_begin;
    Instant t2_begin;
    Instant t3_begin;
    Instant t4_begin;

    auto t1 = [&] () -> void {
        t1_begin = now();
        std::this_thread::sleep_for(210ms);
    };
    auto t2 = [&] () -> void {
        t2_begin = now();
        std::this_thread::sleep_for(300ms);
    };
    auto t3 = [&] () -> void {
        t3_begin = now();
        std::this_thread::sleep_for(255ms);
    };
    auto t4 = [&] () -> void {
        t4_begin = now();
        std::this_thread::sleep_for(10ms);
    };

    Pool pool(1);

    pool.schedule(t1);
    pool.schedule(t2);
    auto f2 = pool.schedule(t4, Task::MinPriority);
    auto f1 = pool.schedule(t3, Task::MaxPriority);

    f2.wait();
    ASSERT_EQ(f1.wait_for(0ms), std::future_status::ready);

    ASSERT_TRUE(t3_begin < t4_begin);
}

TEST(Pool, SharedCallable) {
    class Task {
        public:
            Task(int fv) : mFinalValue(fv) {}
            void operator()() {
                mValue = mFinalValue;
            }
            int value() const {
                return mValue;
            }
        private:
            int mFinalValue;
            int mValue = 0;
    };

    Pool pool;
    auto shh = SharedCallable<Task>(123);

    auto shh_f = pool.schedule(shh);
    shh_f.wait();
    ASSERT_EQ(123, shh->value());
}
