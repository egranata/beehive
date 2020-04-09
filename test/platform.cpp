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
#include <beehive/worker.h>
#include <beehive/platform.h>
#include "gtest/gtest.h"

using namespace beehive;

TEST(Platform, NumCores) {
    ASSERT_TRUE(Platform::numprocessors() > 0);
}

TEST(Platform, Affinity) {
    Pool pool(3);
    std::vector<bool> b;
    b.push_back(true);
    for(size_t i = 1; i < Platform::numprocessors(); ++i) b.push_back(false);
    pool.worker(0).affinity(b);
    std::vector<bool> c = pool.worker(0).affinity();
    ASSERT_TRUE(c.size() > 0);
    ASSERT_TRUE(c[0]);
    for (size_t i = 1; i < c.size(); ++i) ASSERT_FALSE(c[i]);
}
