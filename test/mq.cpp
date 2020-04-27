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
#include <beehive/message.h>
#include "gtest/gtest.h"
#include <thread>
#include <chrono>

using namespace beehive;
using namespace std::chrono_literals;

TEST(Message, Equality) {
    Message m1(Message::EXIT_Data{});
    Message m2(Message::TASK_Data{});
    Message m3(Message::EXIT_Data{});
    ASSERT_EQ(m1, m3);
    ASSERT_NE(m2, m3);
    ASSERT_NE(m1, m2);
}

TEST(MessageQueue, Empty) {
    MessageQueue<Message> mq;
    ASSERT_TRUE(mq.empty());
    ASSERT_EQ(std::nullopt, mq.receive());
}

TEST(MessageQueue, SendReceive) {
    MessageQueue<Message> mq;
    mq.send({Message::EXIT_Data{}});
    auto msg = mq.receive();
    ASSERT_TRUE(msg.has_value());
    ASSERT_EQ(Message::Kind::EXIT, msg->kind());
    ASSERT_NE(std::nullopt, msg->exit());
}

TEST(MessageQueue, ReceiveNull) {
    MessageQueue<Message> mq;
    mq.send({Message::EXIT_Data{}});
    ASSERT_TRUE(mq.receive());
}

TEST(MessageQueue, ReceiveWhatYouSend) {
    MessageQueue<Message> mq;
    mq.send({Message::EXIT_Data{}});
    ASSERT_TRUE(mq.receive());
    ASSERT_FALSE(mq.receive());
}

TEST(MessageQueue, ReceiveInOrder) {
    MessageQueue<Message> mq;
    mq.send({Message::EXIT_Data{}});
    mq.send({Message::TASK_Data{}});
    auto m1 = mq.receive();
    auto m2 = mq.receive();
    ASSERT_TRUE(m1);
    ASSERT_EQ(Message::Kind::EXIT, m1->kind());
    ASSERT_NE(std::nullopt, m1->exit());
    ASSERT_EQ(std::nullopt, m1->task());
    ASSERT_TRUE(m2);
    ASSERT_EQ(Message::Kind::TASK, m2->kind());
    ASSERT_NE(std::nullopt, m2->task());
    ASSERT_EQ(std::nullopt, m2->exit());
}

TEST(SignalingQueue, SendReceive) {
    SignalingQueue<Message> sq;
    std::thread t1([&sq] () -> void {
        std::this_thread::sleep_for(500ms);
        sq.send({Message::TASK_Data{}});
    });
    std::optional<Message> m;
    std::thread t2([&sq, &m] () -> void {
        m = sq.receive();
    });
    t2.join();
    ASSERT_EQ(Message::Kind::TASK, m->kind());
    t1.join();
}

TEST(SignalingQueue, ReceiveTwo) {
    SignalingQueue<Message> sq;
    std::optional<Message> m1;
    std::optional<Message> m2;
    std::thread t1([&sq] () -> void {
        std::this_thread::sleep_for(500ms);
        sq.send({Message::TASK_Data{}});
        std::this_thread::sleep_for(500ms);
        sq.send({Message::EXIT_Data{}});
    });
    std::thread t2([&sq, &m1, &m2] () -> void {
        m1 = sq.receive();
        m2 = sq.receive();
    });
    t2.join();
    ASSERT_EQ(Message::Kind::TASK, m1->kind());
    ASSERT_EQ(Message::Kind::EXIT, m2->kind());
    t1.join();
}

TEST(SignalingQueue, Loop) {
    class TestHandler : public Message::Handler {
        public:
            using R = Message::Handler::Result;

            void onBeforeMessage() override {
                ++mBefores;
            }

            void onAfterMessage() override {
                ++mAfters;
            }

            R onNop(const Message::NOP_Data& p) override {
                ++mNops;
                return Message::Handler::onNop(p);
            }
            R onTask(const Message::TASK_Data& p) override {
                ++mTasks;
                return Message::Handler::onTask(p);
            }
            R onExit(const Message::EXIT_Data& p) override {
                ++mExits;
                return Message::Handler::onExit(p);
            }
            R onDump(const Message::DUMP_Data& p) override {
                ++mDumps;
                return Message::Handler::onDump(p);
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
    SignalingQueue<Message> sq;
    TestHandler th;
    std::thread t1([&sq, &th] () -> void {
        sq.loop(&th);
    });

    sq.send({Message::TASK_Data{}});
    sq.send({Message::TASK_Data{}});
    sq.send({Message::NOP_Data{}});
    sq.send({Message::DUMP_Data{}});
    sq.send({Message::NOP_Data{}});
    sq.send({Message::TASK_Data{}});
    sq.send({Message::EXIT_Data{}});
    t1.join();

    ASSERT_EQ(7, th.befores());
    ASSERT_EQ(7, th.afters());

    ASSERT_EQ(3, th.tasks());
    ASSERT_EQ(2, th.nops());
    ASSERT_EQ(1, th.dumps());
    ASSERT_EQ(1, th.exits());
}

TEST(SignalingQueue, HandlerThread) {
    class TestHandler : public HandlerThread<Message> {
        public:
            using R = Message::Handler::Result;

            void onStart() override {
                ++mOnStart;
            }

            void onBeforeMessage() override {
                ++mBefores;
            }

            void onAfterMessage() override {
                ++mAfters;
            }

            R onNop(const Message::NOP_Data& p) override {
                ++mNops;
                return Message::Handler::onNop(p);
            }
            R onTask(const Message::TASK_Data& p) override {
                ++mTasks;
                return Message::Handler::onTask(p);
            }
            R onExit(const Message::EXIT_Data& p) override {
                ++mExits;
                return Message::Handler::onExit(p);
            }
            R onDump(const Message::DUMP_Data& p) override {
                ++mDumps;
                return Message::Handler::onDump(p);
            }

            size_t starts() const { return mOnStart; }

            size_t befores() const { return mBefores; }
            size_t afters() const { return mAfters; }

            size_t nops() const { return mNops; }
            size_t dumps() const { return mDumps; }
            size_t exits() const { return mExits; }
            size_t tasks() const { return mTasks; }

        private:
            size_t mOnStart = 0;

            size_t mBefores = 0;
            size_t mAfters = 0;

            size_t mNops = 0;
            size_t mDumps = 0;
            size_t mExits = 0;
            size_t mTasks = 0;
    };

    TestHandler th;

    th.queue()->send({Message::TASK_Data{}});
    th.queue()->send({Message::TASK_Data{}});
    th.queue()->send({Message::NOP_Data{}});
    th.queue()->send({Message::DUMP_Data{}});
    th.queue()->send({Message::NOP_Data{}});
    th.queue()->send({Message::TASK_Data{}});
    th.queue()->send({Message::EXIT_Data{}});
    th.join();

    ASSERT_EQ(1, th.starts());

    ASSERT_EQ(7, th.befores());
    ASSERT_EQ(7, th.afters());

    ASSERT_EQ(3, th.tasks());
    ASSERT_EQ(2, th.nops());
    ASSERT_EQ(1, th.dumps());
    ASSERT_EQ(1, th.exits());
}
