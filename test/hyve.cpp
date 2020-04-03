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

#include <beehive/beehive.h>
#include "gtest/gtest.h"
#include <string>
#include <functional>

using namespace beehive;

TEST(HyveTest, RunOneTask) {
    Beehive beehive;
    auto future = beehive.schedule([] (int x) -> int {
        return 2 * x + 3;
    }, 3);
    ASSERT_EQ(9, future.get());
}

TEST(HyveTest, RunTwoTasks) {
    Beehive beehive;
    auto ifuture = beehive.schedule([] (int x) -> int {
        return 2 * x + 3;
    }, 4);
    auto sfuture = beehive.schedule([] (const std::string& s) -> int {
        return s.size();
    }, "hello");
    ASSERT_EQ(11, ifuture.get());
    ASSERT_EQ(5, sfuture.get());
}

TEST(HyveTest, RunDifferentTasks) {
    Beehive beehive;
    auto f1 = beehive.schedule([] (int x, int y) -> int {
        return x + y;
    }, 3, 4);
    auto f2 = beehive.schedule([] (int x) -> int {
        return x - 1;
    }, 7);
    ASSERT_EQ(7, f1.get());
    ASSERT_EQ(6, f2.get());
}

TEST(HyveTest, ScheduleFunction) {
    std::function<int(int)> op = [] (int x) -> int { return x; };
    Beehive beehive;
    auto f = beehive.schedule(op, 42);
    ASSERT_EQ(42, f.get());
}

TEST(HyveTest, Schedule0Arg) {
    Beehive beehive;
    auto f = beehive.schedule([] () -> int { return 1; });
    ASSERT_EQ(1, f.get());
}

TEST(HyveTest, ScheduleVoid) {
    Beehive beehive;
    auto f = beehive.schedule([] () -> void { });
    f.wait();
}

TEST(HyveTest, Metrics) {
    Beehive beehive;
    for (size_t i = 0; i < 20; ++i) {
        auto f = beehive.schedule([] () -> void {});
        f.wait();
    }
    auto stats = beehive.stats();
    uint64_t messages = 0;
    uint64_t tasks = 0;
    std::for_each(stats.begin(), stats.end(), [&messages, &tasks] (const auto& kv) -> void {
        messages += kv.second.messages;
        tasks += kv.second.runs;
    });
    ASSERT_EQ(20, tasks);
    ASSERT_TRUE(messages >= 20);
}
