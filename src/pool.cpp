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

#include <beehive/pool.h>
#include <algorithm>

using namespace beehive;

Pool::Pool(size_t num) {
    if (num == 0) num = std::thread::hardware_concurrency();
    for(int i = 0; i < num; ++i) {
        mWorkers.emplace_back(std::make_unique<Worker>(this, i));
    }
}

Pool::~Pool() = default;

void Pool::foreachworker(std::function<void(std::unique_ptr<Worker>&)> f) {
    std::unique_lock<std::recursive_mutex> lkk(mWorkersMutex);
    std::for_each(mWorkers.begin(), mWorkers.end(), f);
}

Worker* Pool::at(size_t i) const {
    std::unique_lock<std::recursive_mutex> lkk(mWorkersMutex);

    if (i >= mWorkers.size()) return nullptr;
    return mWorkers.at(i).get();
}

size_t Pool::size() const {
    std::unique_lock<std::recursive_mutex> lkk(mWorkersMutex);

    return mWorkers.size();
}

std::shared_future<void> Pool::schedule(Task::Callable c, Task::Priority p) {
    auto tsk = std::make_shared<Task>(c);
    mTasks.push(p, tsk);
    foreachworker([] (std::unique_ptr<Worker>& wb) -> void {
        wb->task();
    });
    return tsk->future();
}

std::vector<Worker::Stats> Pool::stats() {
    std::vector<Worker::Stats> s;
    foreachworker([&s] (std::unique_ptr<Worker>& wb) -> void {
        s.emplace_back(wb->stats());
    });
    return s;
}

Worker::View Pool::worker(int i) {
    std::unique_lock<std::recursive_mutex> lkk(mWorkersMutex);

    if (i >= 0 && i < mWorkers.size()) {
        return mWorkers.at(i)->view();
    } else {
        return Worker::View::empty();
    }
}

bool Pool::idle() const {
    return mTasks.empty();
}

std::shared_ptr<Task> Pool::task() {
    auto tsk = mTasks.trypop();
    return tsk.value_or(nullptr);
}

void Pool::dump() {
    foreachworker([] (std::unique_ptr<Worker>& wb) -> void {
        wb->dump();
    });
}

void Pool::addworker() {
    std::unique_lock<std::recursive_mutex> lkk(mWorkersMutex);

    auto i = mWorkers.size();
    mWorkers.emplace_back(std::make_unique<Worker>(this, i));
}
