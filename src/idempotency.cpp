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

#include <beehive/idempotency.h>

using namespace beehive;

bool IdempotencySet::needsrun(const std::string& id) {
    std::unique_lock<std::mutex> lk(mMutex);

    if (mKeys.count(id) == 0) {
        mKeys.emplace(id);
        return true;
    } else return false;
}
