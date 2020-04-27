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

#include <beehive/message.h>

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

Message::Handler::Handler() = default;
Message::Handler::~Handler() = default;

void Message::Handler::onBeforeMessage() {}
void Message::Handler::onAfterMessage() {}

Message::Handler::Result Message::Handler::handle(const Message& msg) {
    Result r = Result::CONTINUE;

    onBeforeMessage();

    switch (msg.kind()) {
        case Message::Kind::NOP:
            r = onNop(*msg.nop());
            break;
        case Message::Kind::EXIT:
            r = onExit(*msg.exit());
            break;
        case Message::Kind::DUMP:
            r = onDump(*msg.dump());
            break;
        case Message::Kind::TASK:
            r = onTask(*msg.task());
            break;
    }

    onAfterMessage();

    return r;
}

Message::Handler::Result Message::Handler::onNop(const NOP_Data&) {
    return Result::CONTINUE;
}

Message::Handler::Result Message::Handler::onExit(const EXIT_Data&) {
    return Result::FINISH;
}

Message::Handler::Result Message::Handler::onTask(const TASK_Data&) {
    return Result::CONTINUE;
}

Message::Handler::Result Message::Handler::onDump(const DUMP_Data&) {
    return Result::CONTINUE;
}
