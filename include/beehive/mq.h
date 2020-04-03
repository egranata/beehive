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

#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>

namespace beehive {
enum class Message {
    NOP,
    EXIT,
    TASK,
};

class MessageQueue {
    public:
        MessageQueue();

        void send(Message);
        bool empty();
        bool receive(Message*);

    private:
        MessageQueue(const MessageQueue&) = delete;

        std::mutex mQueueMutex;
        std::queue<Message> mQueue;
};

class SignalingQueue {
    public:
        SignalingQueue();

        void send(Message);
        Message receive();
    
    private:
        MessageQueue mQueue;
        std::mutex mWaitMutex;
        std::condition_variable mWaitCV;
};
}
