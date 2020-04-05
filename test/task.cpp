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

#include <beehive/task.h>
#include "gtest/gtest.h"

using namespace beehive;

TEST(Task, Execution) {
    int n = 0;
    Task t([&n] () -> void {
        n = 1;
    });
    t.run();
    ASSERT_EQ(1, n);
}
