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
#include <functional>

using namespace beehive;
using namespace std::chrono_literals;

Message::Message(Kind k) : mKind(k) {}

Message::Kind Message::kind() const {
    return mKind;
}

bool Message::operator==(const Message& rhs) const {
    return kind() == rhs.kind();
}

bool Message::operator!=(const Message& rhs) const {
    return !(*this == rhs);
}

MessageQueue::MessageQueue() = default;

void MessageQueue::send(Message m) {
    std::unique_lock<std::mutex> lk(mQueueMutex);

    mQueue.push(m);
}

bool MessageQueue::empty() {
    std::unique_lock<std::mutex> lk(mQueueMutex);

    return mQueue.empty();
}

bool MessageQueue::receive(Message* m) {
    std::unique_lock<std::mutex> lk(mQueueMutex);

    if (mQueue.empty()) return false;
    if (m) *m = mQueue.front();
    mQueue.pop();
    return true;
}

SignalingQueue::SignalingQueue() = default;

void SignalingQueue::send(Message m) {
    mQueue.send(m);
    mWaitCV.notify_all();
}

Message SignalingQueue::receive() {
loop:
    while(mQueue.empty()) {
        std::unique_lock<std::mutex> lk(mWaitMutex);
        mWaitCV.wait_for(lk, 100ms);
    }
    Message m;
    if (!mQueue.receive(&m)) goto loop;
    return m;
}

void SignalingQueue::loop(Handler* h) {
    Handler::Result r = Handler::Result::CONTINUE;
    while (r == Handler::Result::CONTINUE) {
        auto msg = receive();
        h->onBeforeMessage();
        switch (msg.kind()) {
            case Message::Kind::NOP: {
                r = h->onNop();
            } break;
            case Message::Kind::EXIT: {
                r = h->onExit();
            } break;
            case Message::Kind::DUMP: {
                r = h->onDump();
            } break;
            case Message::Kind::TASK: {
                r = h->onTask();
            } break;
        }
        h->onAfterMessage();
    }
}

SignalingQueue::Handler::Handler() = default;
SignalingQueue::Handler::~Handler() = default;

void SignalingQueue::Handler::onBeforeMessage() {}
void SignalingQueue::Handler::onAfterMessage() {}

SignalingQueue::Handler::Result SignalingQueue::Handler::onNop() {
    return Result::CONTINUE;
}

SignalingQueue::Handler::Result SignalingQueue::Handler::onExit() {
    return Result::FINISH;
}

SignalingQueue::Handler::Result SignalingQueue::Handler::onTask() {
    return Result::CONTINUE;
}

SignalingQueue::Handler::Result SignalingQueue::Handler::onDump() {
    return Result::CONTINUE;
}

SignalingQueue::HandlerThread::HandlerThread() {
    mQueue = std::make_unique<SignalingQueue>();
    mThread = std::thread(std::bind(&HandlerThread::loop, this));
}

void SignalingQueue::HandlerThread::join() {
    mThread.join();
}
void SignalingQueue::HandlerThread::detach() {
    mThread.detach();
}

SignalingQueue* SignalingQueue::HandlerThread::queue() {
    return mQueue.get();
}

void SignalingQueue::HandlerThread::loop() {
    mQueue->loop(this);
}
