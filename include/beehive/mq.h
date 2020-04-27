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

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <optional>
#include <variant>
#include <functional>

namespace beehive {

template<typename MsgType>
class MessageQueue {
    public:
        MessageQueue() = default;
        ~MessageQueue() = default;

        void send(MsgType msg) {
            std::unique_lock<std::mutex> lk(mQueueMutex);
            mQueue.push(msg);
        }

        bool empty() {
            std::unique_lock<std::mutex> lk(mQueueMutex);
            return mQueue.empty();
        }

        std::optional<MsgType> receive() {
            std::unique_lock<std::mutex> lk(mQueueMutex);

            if (mQueue.empty()) return std::nullopt;

            auto msg = mQueue.front();
            mQueue.pop();
            return msg;
        }

    private:
        MessageQueue(const MessageQueue&) = delete;

        std::mutex mQueueMutex;
        std::queue<MsgType> mQueue;
};

template<typename MsgType>
class SignalingQueue {
    public:
        enum class Result {
            CONTINUE,
            ERROR,
            FINISH,
        };

        SignalingQueue() = default;
        ~SignalingQueue() = default;

        void send(MsgType m) {
            mQueue.send(m);
            mWaitCV.notify_all();
        }

        MsgType receive() {
            loop:
                while(mQueue.empty()) {
                    std::unique_lock<std::mutex> lk(mWaitMutex);
                    mWaitCV.wait_for(lk, std::chrono::milliseconds(100));
                }
                std::optional<MsgType> m = mQueue.receive();
                if (m.has_value()) return *m;
                goto loop;
        }
    
        void loop(typename MsgType::Handler* h) {
            auto r = Result::CONTINUE;
            while (r == Result::CONTINUE) {
                auto msg = receive();
                r = h->handle(msg);
            }
        }

    private:
        MessageQueue<MsgType> mQueue;
        std::mutex mWaitMutex;
        std::condition_variable mWaitCV;
};

template<typename MsgType>
class HandlerThread : public MsgType::Handler {
    public:
        using SQ = SignalingQueue<MsgType>;

        HandlerThread() : mQueue(std::make_unique<SQ>()) {
            mThread = std::thread(std::bind(&HandlerThread::loop, this));
        }

        SQ* queue() {
            return mQueue.get();
        }

        void join() {
            mThread.join();
        }
        void detach() {
            mThread.detach();
        }
    protected:
        virtual void onStart() {}

    private:
        void loop() {
            onStart();
            mQueue->loop(this);
        }

        std::unique_ptr<SQ> mQueue;
        std::thread mThread;
};
}
