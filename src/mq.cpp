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

Message::Message(NOP_Data p) : mKind(Message::Kind::NOP), mPayload(p) {}
Message::Message(EXIT_Data p) : mKind(Message::Kind::EXIT), mPayload(p) {}
Message::Message(TASK_Data p) : mKind(Message::Kind::TASK), mPayload(p) {}
Message::Message(DUMP_Data p) : mKind(Message::Kind::DUMP), mPayload(p) {}

Message::Kind Message::kind() const {
    return mKind;
}

std::optional<Message::NOP_Data> Message::nop() const {
    if (std::holds_alternative<NOP_Data>(mPayload)) return std::get<NOP_Data>(mPayload);
    else return std::nullopt;
}
std::optional<Message::EXIT_Data> Message::exit() const {
    if (std::holds_alternative<EXIT_Data>(mPayload)) return std::get<EXIT_Data>(mPayload);
    else return std::nullopt;
}
std::optional<Message::TASK_Data> Message::task() const {
    if (std::holds_alternative<TASK_Data>(mPayload)) return std::get<TASK_Data>(mPayload);
    else return std::nullopt;
}
std::optional<Message::DUMP_Data> Message::dump() const {
    if (std::holds_alternative<DUMP_Data>(mPayload)) return std::get<DUMP_Data>(mPayload);
    else return std::nullopt;
}

bool Message::operator==(const Message& rhs) const {
    if (mKind != rhs.mKind) return false;
    if (mPayload.index() != rhs.mPayload.index()) return false;

    return (nop() == rhs.nop()) &&
           (exit() == rhs.exit()) &&
           (task() == rhs.task()) &&
           (dump() == rhs.dump());
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

std::optional<Message> MessageQueue::receive() {
    std::unique_lock<std::mutex> lk(mQueueMutex);

    if (mQueue.empty()) return std::nullopt;

    auto msg = mQueue.front();
    mQueue.pop();
    return msg;
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
    std::optional<Message> m = mQueue.receive();
    if (m.has_value()) return *m;
    goto loop;
}

void SignalingQueue::loop(Handler* h) {
    Handler::Result r = Handler::Result::CONTINUE;
    while (r == Handler::Result::CONTINUE) {
        auto msg = receive();
        h->onBeforeMessage();
        switch (msg.kind()) {
            case Message::Kind::NOP: {
                r = h->onNop(*msg.nop());
            } break;
            case Message::Kind::EXIT: {
                r = h->onExit(*msg.exit());
            } break;
            case Message::Kind::DUMP: {
                r = h->onDump(*msg.dump());
            } break;
            case Message::Kind::TASK: {
                r = h->onTask(*msg.task());
            } break;
        }
        h->onAfterMessage();
    }
}

SignalingQueue::Handler::Handler() = default;
SignalingQueue::Handler::~Handler() = default;

void SignalingQueue::Handler::onBeforeMessage() {}
void SignalingQueue::Handler::onAfterMessage() {}

SignalingQueue::Handler::Result SignalingQueue::Handler::onNop(const Message::NOP_Data&) {
    return Result::CONTINUE;
}

SignalingQueue::Handler::Result SignalingQueue::Handler::onExit(const Message::EXIT_Data&) {
    return Result::FINISH;
}

SignalingQueue::Handler::Result SignalingQueue::Handler::onTask(const Message::TASK_Data&) {
    return Result::CONTINUE;
}

SignalingQueue::Handler::Result SignalingQueue::Handler::onDump(const Message::DUMP_Data&) {
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
