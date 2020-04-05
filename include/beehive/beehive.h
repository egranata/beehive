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
#include <memory>
#include <type_traits>

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

        Pool* operator->() {
            return &mPool;
        }

    private:
        Pool mPool;
};
}
