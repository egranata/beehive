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
        Message m = mMsgQueue.receive();
        mStats.message();
        switch (m) {
            case Message::NOP: break;
            case Message::EXIT: return;
            case Message::TASK: {
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
    send(Message::EXIT);
}

void Worker::task() {
    send(Message::TASK);
}

Worker::AtomicStats::AtomicStats() = default;

Worker::Stats Worker::AtomicStats::load() {
    Stats s;
    s.messages = mMessages.load();
    s.runs = mRuns.load();
    return s;
}

void Worker::AtomicStats::message() {
    mMessages.fetch_add(1);
}

void Worker::AtomicStats::run() {
    mRuns.fetch_add(1);
}
