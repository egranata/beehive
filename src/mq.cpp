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

using namespace beehive;

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
        mWaitCV.wait(lk);
    }
    Message m;
    if (!mQueue.receive(&m)) goto loop;
    return m;
}


