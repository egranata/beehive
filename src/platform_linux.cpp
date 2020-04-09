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

#ifdef __linux__

#include <beehive/platform.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <type_traits>

static_assert(std::is_same_v<std::thread::native_handle_type, pthread_t>);

using namespace beehive;

size_t Platform::numprocessors() {
    return get_nprocs();
}

std::vector<bool> Platform::affinity(std::thread::native_handle_type nh) {
    std::vector<bool> ret;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    int ok = pthread_getaffinity_np(nh, sizeof(cpu_set_t), &cpuset);
    if (ok == 0) {
        for (auto i = 0; i < CPU_SETSIZE; i++) {
            if (CPU_ISSET(i, &cpuset)) ret.push_back(true);
            else ret.push_back(false);
        }
    }
    return ret;
}
void Platform::affinity(std::thread::native_handle_type nh, const std::vector<bool>& aff) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (auto i = 0; i < aff.size(); ++i) {
        if (aff[i]) CPU_SET(i, &cpuset);
    }
    pthread_setaffinity_np(nh, sizeof(cpu_set_t), &cpuset);
}

#endif
