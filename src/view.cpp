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

#include <beehive/worker.h>

using namespace beehive;

Worker::View Worker::View::empty() {
    return View(nullptr);
}

Worker::View::View(Worker* wk) : mWorker(wk) {}

Worker::View::operator bool() const {
    return (mWorker != nullptr);
}

const char* Worker::View::name() const {
    return mWorker->name();
}

void Worker::View::name(const char* n) {
    mWorker->name(n);
}

int Worker::View::id() const {
    return mWorker->id();
}

std::thread::id Worker::View::tid() {
    return mWorker->tid();
}

std::thread::native_handle_type Worker::View::nativeid() {
    return mWorker->nativeid();
}

Worker::Stats Worker::View::stats() {
    return mWorker->stats();
}

std::vector<bool> Worker::View::affinity() {
    return mWorker->affinity();
}
void Worker::View::affinity(const std::vector<bool>& a) {
    mWorker->affinity(a);
}

