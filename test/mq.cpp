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

#include <beehive/mq.h>
#include "gtest/gtest.h"
#include <thread>
#include <chrono>

using namespace beehive;
using namespace std::chrono_literals;

TEST(MessageQueue, Empty) {
    MessageQueue mq;
    ASSERT_TRUE(mq.empty());
    ASSERT_FALSE(mq.receive(nullptr));
}

TEST(MessageQueue, SendReceive) {
    MessageQueue mq;
    mq.send(Message::EXIT);
    Message m;
    ASSERT_TRUE(mq.receive(&m));
    ASSERT_EQ(Message::EXIT, m);
}

TEST(MessageQueue, ReceiveNull) {
    MessageQueue mq;
    mq.send(Message::EXIT);
    ASSERT_TRUE(mq.receive(nullptr));
}

TEST(MessageQueue, ReceiveWhatYouSend) {
    MessageQueue mq;
    mq.send(Message::EXIT);
    ASSERT_TRUE(mq.receive(nullptr));
    ASSERT_FALSE(mq.receive(nullptr));
}

TEST(MessageQueue, ReceiveInOrder) {
    MessageQueue mq;
    mq.send(Message::EXIT);
    mq.send(Message::TASK);
    Message m;
    ASSERT_TRUE(mq.receive(&m));
    ASSERT_EQ(Message::EXIT, m);
    ASSERT_TRUE(mq.receive(&m));
    ASSERT_EQ(Message::TASK, m);
}

TEST(SignalingQueue, SendReceive) {
    SignalingQueue sq;
    Message m = Message::NOP;
    std::thread t1([&sq] () -> void {
        std::this_thread::sleep_for(500ms);
        sq.send(Message::TASK);
    });
    std::thread t2([&sq, &m] () -> void {
        m = sq.receive();
    });
    t2.join();
    ASSERT_EQ(Message::TASK, m);
    t1.join();
}

TEST(SignalingQueue, ReceiveTwo) {
    SignalingQueue sq;
    Message m1 = Message::NOP;
    Message m2 = Message::NOP;
    std::thread t1([&sq] () -> void {
        std::this_thread::sleep_for(500ms);
        sq.send(Message::TASK);
        std::this_thread::sleep_for(500ms);
        sq.send(Message::EXIT);
    });
    std::thread t2([&sq, &m1, &m2] () -> void {
        m1 = sq.receive();
        m2 = sq.receive();
    });
    t2.join();
    ASSERT_EQ(Message::TASK, m1);
    ASSERT_EQ(Message::EXIT, m2);
    t1.join();
}
