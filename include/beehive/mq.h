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
#include <thread>

namespace beehive {
class Message {
    public:
        enum class Kind {
            NOP,
            EXIT,
            TASK,
            DUMP,
        };
        Message(Kind k = Kind::NOP);

        Kind kind() const;
        bool operator==(const Message& rhs) const;
        bool operator!=(const Message& rhs) const;
    private:
        Kind mKind;
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
        class Handler {
            public:
                enum class Result {
                    CONTINUE,
                    ERROR,
                    FINISH,
                };

                virtual void onBeforeMessage();
                virtual void onAfterMessage();

                virtual Result onNop();
                virtual Result onExit();
                virtual Result onTask();
                virtual Result onDump();
            protected:
                Handler();
                virtual ~Handler();
        };

        class HandlerThread : public Handler {
            public:
                HandlerThread();

                SignalingQueue* queue();

                void join();
                void detach();
            private:
                void loop();

                std::unique_ptr<SignalingQueue> mQueue;
                std::thread mThread;
        };

        SignalingQueue();

        void send(Message);
        Message receive();
    
        void loop(Handler*);

    private:
        MessageQueue mQueue;
        std::mutex mWaitMutex;
        std::condition_variable mWaitCV;
};
}
