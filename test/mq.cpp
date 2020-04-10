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

TEST(Message, Equality) {
    Message m1(Message::Kind::EXIT);
    Message m2(Message::Kind::TASK);
    Message m3(Message::Kind::EXIT);
    ASSERT_EQ(m1, m3);
    ASSERT_NE(m2, m3);
    ASSERT_NE(m1, m2);
}

TEST(MessageQueue, Empty) {
    MessageQueue mq;
    ASSERT_TRUE(mq.empty());
    ASSERT_FALSE(mq.receive(nullptr));
}

TEST(MessageQueue, SendReceive) {
    MessageQueue mq;
    mq.send(Message::Kind::EXIT);
    Message m;
    ASSERT_TRUE(mq.receive(&m));
    ASSERT_EQ(Message::Kind::EXIT, m.kind());
}

TEST(MessageQueue, ReceiveNull) {
    MessageQueue mq;
    mq.send(Message::Kind::EXIT);
    ASSERT_TRUE(mq.receive(nullptr));
}

TEST(MessageQueue, ReceiveWhatYouSend) {
    MessageQueue mq;
    mq.send(Message::Kind::EXIT);
    ASSERT_TRUE(mq.receive(nullptr));
    ASSERT_FALSE(mq.receive(nullptr));
}

TEST(MessageQueue, ReceiveInOrder) {
    MessageQueue mq;
    mq.send(Message::Kind::EXIT);
    mq.send(Message::Kind::TASK);
    Message m;
    ASSERT_TRUE(mq.receive(&m));
    ASSERT_EQ(Message::Kind::EXIT, m.kind());
    ASSERT_TRUE(mq.receive(&m));
    ASSERT_EQ(Message::Kind::TASK, m.kind());
}

TEST(SignalingQueue, SendReceive) {
    SignalingQueue sq;
    Message m = Message::Kind::NOP;
    std::thread t1([&sq] () -> void {
        std::this_thread::sleep_for(500ms);
        sq.send(Message::Kind::TASK);
    });
    std::thread t2([&sq, &m] () -> void {
        m = sq.receive();
    });
    t2.join();
    ASSERT_EQ(Message::Kind::TASK, m.kind());
    t1.join();
}

TEST(SignalingQueue, ReceiveTwo) {
    SignalingQueue sq;
    Message m1 = Message::Kind::NOP;
    Message m2 = Message::Kind::NOP;
    std::thread t1([&sq] () -> void {
        std::this_thread::sleep_for(500ms);
        sq.send(Message::Kind::TASK);
        std::this_thread::sleep_for(500ms);
        sq.send(Message::Kind::EXIT);
    });
    std::thread t2([&sq, &m1, &m2] () -> void {
        m1 = sq.receive();
        m2 = sq.receive();
    });
    t2.join();
    ASSERT_EQ(Message::Kind::TASK, m1.kind());
    ASSERT_EQ(Message::Kind::EXIT, m2.kind());
    t1.join();
}

TEST(SignalingQueue, Loop) {
    class TestHandler : public SignalingQueue::Handler {
        public:
            using R = SignalingQueue::Handler::Result;

            void onBeforeMessage() override {
                ++mBefores;
            }

            void onAfterMessage() override {
                ++mAfters;
            }

            R onNop() override {
                ++mNops;
                return SignalingQueue::Handler::onNop();
            }
            R onTask() override {
                ++mTasks;
                return SignalingQueue::Handler::onTask();
            }
            R onExit() override {
                ++mExits;
                return SignalingQueue::Handler::onExit();
            }
            R onDump() override {
                ++mDumps;
                return SignalingQueue::Handler::onDump();
            }

            size_t befores() const { return mBefores; }
            size_t afters() const { return mAfters; }

            size_t nops() const { return mNops; }
            size_t dumps() const { return mDumps; }
            size_t exits() const { return mExits; }
            size_t tasks() const { return mTasks; }

        private:
            size_t mBefores = 0;
            size_t mAfters = 0;

            size_t mNops = 0;
            size_t mDumps = 0;
            size_t mExits = 0;
            size_t mTasks = 0;
    };
    SignalingQueue sq;
    TestHandler th;
    std::thread t1([&sq, &th] () -> void {
        sq.loop(&th);
    });

    sq.send(Message::Kind::TASK);
    sq.send(Message::Kind::TASK);
    sq.send(Message::Kind::NOP);
    sq.send(Message::Kind::DUMP);
    sq.send(Message::Kind::NOP);
    sq.send(Message::Kind::TASK);
    sq.send(Message::Kind::EXIT);
    t1.join();

    ASSERT_EQ(7, th.befores());
    ASSERT_EQ(7, th.afters());

    ASSERT_EQ(3, th.tasks());
    ASSERT_EQ(2, th.nops());
    ASSERT_EQ(1, th.dumps());
    ASSERT_EQ(1, th.exits());
}
