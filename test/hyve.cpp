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
#include <algorithm>
#include <iterator>
#include <mutex>
#include <string>
#include <functional>
#include <sstream>
#include <iterator>

using namespace beehive;
using namespace std::chrono_literals;

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

TEST(HyveTest, ScheduleObject) {
    Beehive beehive;
    struct F {
        int operator()(const int& n) {
            return n + 1;
        }
    };
    auto f = beehive.schedule(F(), 1);
    ASSERT_EQ(2, f.get());
}

TEST(HyveTest, Metrics) {
    Beehive beehive;
    for (size_t i = 0; i < 20; ++i) {
        auto f = beehive.schedule([] () -> void {});
        f.wait();
    }
    auto stats = beehive->stats();
    uint64_t messages = 0;
    uint64_t tasks = 0;
    std::for_each(stats.begin(), stats.end(), [&messages, &tasks] (const auto& kv) -> void {
        messages += kv.messages;
        tasks += kv.runs;
    });
    ASSERT_EQ(20, tasks);
    ASSERT_TRUE(messages >= 20);
}

TEST(HyveTest, ForEach) {
    Beehive beehive;
    std::vector<int> v0 = {1,2,3,4,5};
    std::vector<std::string> v1;
    std::mutex mtx;
    auto f = [&v1, &mtx] (int x) -> void {
        std::this_thread::sleep_for(100ms);
        x = 2 * x;
        std::stringstream ss;
        ss << x;
        {
            std::unique_lock<std::mutex> lk(mtx);
            v1.push_back(ss.str());
        }
    };
    beehive.foreach(v0.begin(), v0.end(), f);
    ASSERT_EQ(v0.size(), v1.size());

    auto v1_beg = v1.begin();
    auto v1_end = v1.end();

    ASSERT_FALSE(std::find(v1_beg, v1_end, "2") == v1_end);
    ASSERT_FALSE(std::find(v1_beg, v1_end, "4") == v1_end);
    ASSERT_FALSE(std::find(v1_beg, v1_end, "6") == v1_end);
    ASSERT_FALSE(std::find(v1_beg, v1_end, "8") == v1_end);
    ASSERT_FALSE(std::find(v1_beg, v1_end, "10") == v1_end);
}

TEST(HyveTest, Transform) {
    Beehive beehive;
    std::vector<int> v0 = {1,2,3,4,5};
    std::map<int, int> m0;
    auto f = [] (int x) -> std::pair<int, int> {
        std::this_thread::sleep_for(111ms);
        return {x, x + 1};
    };
    beehive.transform(v0.begin(), v0.end(), f, std::inserter(m0, m0.end()));
    ASSERT_EQ(m0.size(), v0.size());

    for (const auto& kv : m0) {
        ASSERT_EQ(kv.first + 1, kv.second);
    }
}
