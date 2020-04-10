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

#include <beehive/timecounter.h>
#include "gtest/gtest.h"
#include <thread>

using namespace beehive;
using namespace std::chrono_literals;

TEST(TimeCounter, StartsAt0) {
    TimeCounter tc;
    ASSERT_TRUE(tc.value() == 0ms);
}

TEST(TimeCounter, Increment) {
    TimeCounter tc;
    tc.start();
    std::this_thread::sleep_for(100ms);
    tc.stop();
    ASSERT_TRUE(tc.value() >= 100ms);
}

TEST(TimeCounter, IncrementTwice) {
    TimeCounter tc;

    tc.start();
    std::this_thread::sleep_for(100ms);
    tc.stop();

    std::this_thread::sleep_for(100ms);

    tc.start();
    std::this_thread::sleep_for(50ms);
    tc.stop();

    ASSERT_TRUE(tc.value() >= 150ms);
}

TEST(TimeCounter, IgnoreDoubleStart) {
    TimeCounter tc;

    tc.start();
    std::this_thread::sleep_for(100ms);
    tc.start();
    std::this_thread::sleep_for(100ms);
    tc.stop();

    ASSERT_TRUE(tc.value() >= 200ms);
}

TEST(TimeCounter, IgnoreDoubleStop) {
    TimeCounter tc;

    tc.start();
    std::this_thread::sleep_for(100ms);
    tc.stop();

    auto val = tc.value();

    std::this_thread::sleep_for(100ms);

    tc.stop();
    ASSERT_EQ(val, tc.value());
}
