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
#include <stdlib.h>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace beehive;
using namespace std::chrono_literals;

TEST(PoolTest, Size) {
    Pool pool(10);
    ASSERT_EQ(10, pool.size());
}

TEST(PoolTest, Idle) {
    Pool pool(1);
    ASSERT_TRUE(pool.idle());
}

TEST(PoolTest, OneTask) {
    Pool pool(1);
    int n = 0;
    auto future = pool.schedule([&n] () -> void {
        n = 1;
    });
    future.wait();
    ASSERT_EQ(1, n);
}

TEST(PoolTest, TwoTasks) {
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

TEST(PoolTest, TwoParallelTasks) {
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

TEST(PoolTest, TwoLongTasks) {
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

TEST(PoolTest, OneTaskRunsOnce) {
    Pool pool(2);
    bool ran = false;
    auto f1 = pool.schedule([&ran] () -> void {
        if (ran) abort();
        ran = true;
    });
    f1.wait();
    ASSERT_EQ(true, ran);
}

TEST(PoolTest, Stats) {
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
        messages += kv.second.messages;
        tasks += kv.second.runs;
    });
    ASSERT_EQ(4, tasks);
    ASSERT_TRUE(messages >= 4);
}

TEST(PoolTest, HyveOfZero) {
    Pool pool;
    ASSERT_TRUE(pool.size() > 0);
}
