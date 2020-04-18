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

using namespace beehive;
using namespace std::chrono_literals;

TEST(Stress, Map10000) {
    constexpr size_t TOTAL_TESTS = 10000;

    class Op {
        public:
            Op(int i) : mInput(i), mOutput(i + 3) {}
            int operator()() {
                return mOutput;
            }

            bool check(int n) {
                return n == mOutput;
            }
        private:
            int mInput;
            int mOutput;
    };

    Beehive beehive;
    std::vector<Op> ops;
    for(int i = 0; i < TOTAL_TESTS; ++i) {
        ops.emplace_back(i);
    }
    std::vector<std::shared_future<int>> futures;
    for (int i = 0; i < TOTAL_TESTS; ++i) {
        futures.push_back(beehive.schedule(ops[i]));
    }
    for(int i = 0; i < TOTAL_TESTS; ++i) {
        auto n = futures[i].get();
        ASSERT_TRUE(ops[i].check(n));
    }
}
