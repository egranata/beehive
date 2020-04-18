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

#include <beehive/pq.h>
#include "gtest/gtest.h"
#include <string>

using namespace beehive;

TEST(PriorityQueue, EmptyQueue) {
    PriorityQueue<int, std::string> pq;
    ASSERT_TRUE(pq.empty());
    ASSERT_EQ(0, pq.size());
}

TEST(PriorityQueue, Peek) {
    PriorityQueue<int, std::string> pq;
    pq.push(123, "hello world");
    ASSERT_FALSE(pq.empty());
    ASSERT_EQ(1, pq.size());
    ASSERT_EQ("hello world", pq.peek());
}

TEST(PriorityQueue, MaxFirst) {
    PriorityQueue<int, std::string> pq;
    pq.push(123, "hello world");
    pq.push(321, "hi there");
    ASSERT_EQ("hi there", pq.peek());
    pq.pop();
    ASSERT_EQ("hello world", pq.peek());
}

TEST(PriorityQueue, FirstItemWins) {
    PriorityQueue<int, int> pq;
    pq.push(100, 200);
    pq.push(300, 50);
    ASSERT_EQ(50, pq.pop());
}

TEST(PriorityQueue, MinFirst) {
    PriorityQueue<int, std::string, MinFirst> pq;
    pq.push(123, "hello world");
    pq.push(321, "hi there");
    ASSERT_EQ("hello world", pq.peek());
    pq.pop();
    ASSERT_EQ("hi there", pq.peek());
}

TEST(PriorityQueue, Priority) {
    PriorityQueue<int, int> pq;
    int p = 0;
    pq.push(100, 200);
    pq.push(300, 50);

    ASSERT_EQ(300, pq.priority());
    pq.pop(&p);
    ASSERT_EQ(300, p);

    ASSERT_EQ(100, pq.priority());
    pq.pop(&p);
    ASSERT_EQ(100, p);
}

TEST(PriorityQueue, TryPop) {
    PriorityQueue<int, int> pq;
    pq.push(100, 200);
    pq.push(300, 50);
    ASSERT_EQ(50, pq.trypop());
    ASSERT_EQ(200, pq.trypop());
    ASSERT_EQ(std::nullopt, pq.trypop());
}
