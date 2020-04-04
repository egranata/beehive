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

#include <beehive/worker.h>
#include <beehive/pool.h>

using namespace beehive;

Worker::Worker(Pool* parent) : mParent(parent) {
    mWorkThread = std::thread([this] {
        this->WorkLoop();
    });
}

void Worker::WorkLoop() {
    while(true) {
        auto sidle = std::chrono::steady_clock::now();
        Message m = mMsgQueue.receive();
        auto eidle = std::chrono::steady_clock::now();
        mStats.idle(sidle, eidle);
        mStats.message();
        auto sactive = std::chrono::steady_clock::now();
        switch (m.kind()) {
            case Message::Kind::NOP: break;
            case Message::Kind::EXIT: return;
            case Message::Kind::TASK: {
                auto task = mParent->task();
                if (task) {
                    mStats.run();
                    auto& callable = task->callable();
                    auto& promise = task->promise();

                    callable();
                    promise.set_value();
                }
            } break;
        }
        auto eactive = std::chrono::steady_clock::now();
        mStats.active(sactive, eactive);
    }
}

Worker::Stats Worker::stats() {
    return mStats.load();
}

std::thread::id Worker::tid() {
    return mWorkThread.get_id();
}

void Worker::send(Message m) {
    mMsgQueue.send(m);
}

Worker::~Worker() {
    exit();
    mWorkThread.join();
}

void Worker::exit() {
    send(Message::Kind::EXIT);
}

void Worker::task() {
    send(Message::Kind::TASK);
}

Worker::AtomicStats::AtomicStats() = default;

Worker::Stats Worker::AtomicStats::load() {
    Stats s;
    s.messages = mMessages.load();
    s.runs = mRuns.load();
    s.idle = mIdle.load();
    s.active = mActive.load();
    return s;
}

void Worker::AtomicStats::message() {
    mMessages.fetch_add(1);
}

void Worker::AtomicStats::run() {
    mRuns.fetch_add(1);
}

void Worker::AtomicStats::active(std::chrono::steady_clock::time_point from, std::chrono::steady_clock::time_point to) {
loop:
    auto o = mActive.load();
    auto n = std::chrono::duration_cast<std::chrono::milliseconds>(o + to - from);
    bool ok = mActive.compare_exchange_weak(o, n);
    if (!ok) goto loop;
}
void Worker::AtomicStats::idle(std::chrono::steady_clock::time_point from, std::chrono::steady_clock::time_point to) {
loop:
    auto o = mIdle.load();
    auto n = std::chrono::duration_cast<std::chrono::milliseconds>(o + to - from);
    bool ok = mIdle.compare_exchange_weak(o, n);
    if (!ok) goto loop;
}
