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

#include <beehive/idempotency.h>
#include "gtest/gtest.h"
#include <thread>
#include <chrono>
#include <beehive/beehive.h>

using namespace beehive;
using namespace std::chrono_literals;

TEST(IdempotencySet, NewTask) {
    IdempotencySet is;

    ASSERT_TRUE(is.needsrun("new task"));
    ASSERT_TRUE(is.needsrun("another new task"));
}

TEST(IdempotencySet, SameTask) {
    IdempotencySet is;

    is.needsrun("task 1");
    ASSERT_FALSE(is.needsrun("task 1"));
    ASSERT_TRUE(is.needsrun("task 2"));
    ASSERT_FALSE(is.needsrun("task 2"));
    ASSERT_FALSE(is.needsrun("task 1"));
}

TEST(IdempotencySet, RunTasks) {
    Beehive bh;
    int counter1 = 12;
    int counter2 = 21;

    auto f1 = bh.schedule([&] () -> void {
        if (bh->idempotency().needsrun("task1")) {
            ++counter1;
        }
    });
    auto f2 = bh.schedule([&] () -> void {
        if (bh->idempotency().needsrun("task1")) {
            ++counter1;
        }
    });

    auto f3 = bh.schedule([&] () -> void {
        if (bh->idempotency().needsrun("task2")) {
            --counter2;
        }
    });
    auto f4 = bh.schedule([&] () -> void {
        if (bh->idempotency().needsrun("task2")) {
            ++counter2;
        }
    });

    f1.wait(); f2.wait();
    f3.wait(); f4.wait();

    ASSERT_EQ(13, counter1);
    ASSERT_EQ(20, counter2);
}
