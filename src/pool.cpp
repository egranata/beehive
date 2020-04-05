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

Worker* Pool::at(size_t i) const {
    if (i >= size()) return nullptr;
    return mWorkers.at(i).get();
}

size_t Pool::size() const {
    return mWorkers.size();
}

std::shared_future<void> Pool::schedule(Task::Callable c) {
    std::unique_lock<std::mutex> lk(mTasksMutex);

    mTasks.push(std::make_shared<Task>(c));
    std::for_each(mWorkers.begin(), mWorkers.end(), [] (std::unique_ptr<Worker>& wb) -> void {
        wb->task();
    });
    return mTasks.back()->future();
}

std::vector<Worker::Stats> Pool::stats() {
    std::vector<Worker::Stats> s;
    for (int i = 0; i < size(); ++i) {
        s.emplace_back(mWorkers.at(i)->stats());
    }
    return s;
}

Worker::View Pool::worker(int i) {
    if (i >= 0 && i < size()) {
        return mWorkers.at(i)->view();
    } else {
        return Worker::View::empty();
    }
}

bool Pool::idle() const {
    std::unique_lock<std::mutex> lk(mTasksMutex);

    return mTasks.empty();
}

std::shared_ptr<Task> Pool::task() {
    std::unique_lock<std::mutex> lk(mTasksMutex);

    if (mTasks.empty()) return nullptr;
    auto task = mTasks.front();
    mTasks.pop();
    return std::move(task);
}

void Pool::dump() {
    std::for_each(mWorkers.begin(), mWorkers.end(), [] (std::unique_ptr<Worker>& wb) -> void {
        wb->dump();
    });
}
