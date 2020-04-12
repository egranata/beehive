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

#include <functional>
#include <future>
#include <beehive/pool.h>
#include <iterator>
#include <memory>
#include <type_traits>
#include <vector>

namespace beehive {
class Beehive {
    public:
        Beehive(size_t n = 0) : mPool(n) {}
        ~Beehive() = default;

        template<class Callable, class... Args>
        std::shared_future<std::result_of_t<Callable(Args...)>> schedule(Callable f, Args... args) {
            using R = std::result_of_t<Callable(Args...)>;
            using Promise = std::promise<R>;
            using Future = std::shared_future<R>;
            auto flower = std::make_shared<Promise>();
            Future pollen = Future(flower->get_future());
            auto task = [args = std::make_tuple(std::forward<Args>(args) ...), f, flower] () -> void {
                if constexpr (std::is_same_v<R, void>) {
                    std::apply(f,args);
                    flower->set_value();
                } else {
                    auto result = std::apply(f, args);
                    flower->set_value(result);
                }
            };
            mPool.schedule(task);
            return pollen;
        }

        template<typename InIter, typename Callable>
        void foreach(InIter from, InIter to, Callable f) {
            using In = typename std::iterator_traits<InIter>::value_type;
            using R = std::result_of_t<Callable(In)>;
            using Future = std::shared_future<R>;
            std::vector<Future> futures;
            for (; from != to; ++from) {
                futures.push_back(schedule(f, *from));
            }
            for (const auto& future : futures) {
                future.wait();
            }
        }

        template<typename InIter, typename Callable, typename OutIter>
        void transform(InIter from, InIter to, Callable f, OutIter dest) {
            using In = typename std::iterator_traits<InIter>::value_type;
            using R = std::result_of_t<Callable(In)>;
            using Future = std::shared_future<R>;
            using Type = std::pair<bool, Future>;
            std::vector<Type> futures;
            for (; from != to; ++from) {
                futures.push_back({false, schedule(f, *from)});
            }
            bool more = true;
            while(more) {
                more = false;
                for (auto& f : futures) {
                    if (f.first) continue;
                    more = true;
                    auto ready = f.second.wait_for(std::chrono::seconds(0));
                    if (ready == std::future_status::ready) {
                        f.first = true;
                        *dest++ = f.second.get();
                    }
                }
            }
        }

        Pool* operator->() {
            return &mPool;
        }

    private:
        Pool mPool;
};
}
