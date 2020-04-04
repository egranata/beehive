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
#include <iostream>
#include <string>

namespace beehive {
enum class LogLevel {
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    FATAL = 5
};
}

namespace {
class NullOstream : public std::ostream {
    public:
        NullOstream() = default;
        ~NullOstream() = default;
        template<typename T>
        NullOstream& operator<<(const T& object) {
            return *this;
        }
};

class LogOstream : public std::ostream {
    public:
        LogOstream(const char* tag, beehive::LogLevel level) : mTag(tag), mLevel(level), mNull() {}
        ~LogOstream() = default;
        std::ostream& operator<<(const beehive::LogLevel ll) {
            if (shouldLog(ll)) {
                std::cerr << mTag << ": ";
                return std::cerr;
            } else {
                return mNull;
            }
        }
    private:
        bool shouldLog(beehive::LogLevel msgLevel) {
            int self = static_cast<int>(mLevel);
            int msg = static_cast<int>(msgLevel);
            return (msg >= self);
        }

        std::string mTag;
        NullOstream mNull;
        beehive::LogLevel mLevel;
};
}

static constexpr auto DEBUG = beehive::LogLevel::DEBUG;
static constexpr auto INFO = beehive::LogLevel::INFO;
static constexpr auto WARNING = beehive::LogLevel::WARNING;
static constexpr auto ERROR = beehive::LogLevel::ERROR;
static constexpr auto FATAL = beehive::LogLevel::FATAL;

#define LOG_DEBUG(TAG) \
    static LogOstream log ## TAG(#TAG, beehive::LogLevel::DEBUG);
#define LOG_INFO(TAG) \
    static LogOstream log ## TAG(#TAG, beehive::LogLevel::INFO);
#define LOG_WARNING(TAG) \
    static LogOstream log ## TAG(#TAG, beehive::LogLevel::WARNING);
#define LOG_ERROR(TAG) \
    static LogOstream log ## TAG(#TAG, beehive::LogLevel::ERROR);
#define LOG_FATAL(TAG) \
    static LogOstream log ## TAG(#TAG, beehive::LogLevel::FATAL);
