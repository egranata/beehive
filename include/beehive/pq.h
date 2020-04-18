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
#include <queue>
#include <vector>
#include <optional>

namespace beehive {
constexpr bool MaxFirst = true;
constexpr bool MinFirst = false;

template<typename Key, typename Value, bool Order = true>
class PriorityQueue {
    using Object = std::pair<Key, Value>;
    public:
        PriorityQueue() = default;
        ~PriorityQueue() = default;

        void push(Key k, Value v) {
            std::unique_lock<std::mutex> lk(mValuesMutex);
            mValues.emplace(k,v);
        }

        bool empty() const {
            std::unique_lock<std::mutex> lk(mValuesMutex);
            return mValues.empty();
        }

        size_t size() const {
            std::unique_lock<std::mutex> lk(mValuesMutex);
            return mValues.size();
        }

        std::optional<Value> trypop(Key* priority = nullptr) {
            std::unique_lock<std::mutex> lk(mValuesMutex);
            if (mValues.empty()) return std::nullopt;
            auto v = mValues.top();
            mValues.pop();
            if (priority) *priority = v.first;
            return v.second;
        }

        Value pop(Key* priority = nullptr) {
            std::unique_lock<std::mutex> lk(mValuesMutex);
            auto v = mValues.top();
            mValues.pop();
            if (priority) *priority = v.first;
            return v.second;
        }

        Key priority() {
            std::unique_lock<std::mutex> lk(mValuesMutex);
            return mValues.top().first;
        }

        Value peek() {
            std::unique_lock<std::mutex> lk(mValuesMutex);
            return mValues.top().second;
        }
    private:
        mutable std::mutex mValuesMutex;

        struct Comparator {
            bool operator()(const Object& lhs, const Object& rhs) const {
                if constexpr (Order)
                    return lhs.first < rhs.first;
                else
                    return lhs.first > rhs.first;
            }
        };
        std::priority_queue<Object, std::vector<Object>, Comparator> mValues;
};
}
