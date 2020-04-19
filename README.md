# The Beehive library

Beehive is a thread pool manager library.

Its goals include:
 - easy to use;
 - modern C++;
 - minimal overhead;
 - emphasis on testing

No real-time performance guarantees are made for this library

# Example

Using Beehive is as simple as

```
#include <beehive/beehive.h>
beehive::Beehive pool;
auto future = pool.schedule([] (int x) -> int {
    return 2 * x + 3;
}, 3);
...
std::cout << "The result is " << future.get() << std::endl; // prints 9
```

# Features

Beehive features include:
 - dynamic addition of worker threads;
 - task priorities;
 - functional APIs.

# Design

At the core of Beehive there is a managed set of threads and an event-based message queue.  
Worker threads pull messages from the queue (and otherwise sleep most of the time), and execute on tasks as needed.

A `Pool` object manages the threads' lifetime, and keeps synchronization and performance metrics among them.  
A user-friendly API wrapper (the `Beehive` object) is provided on top of the `Pool` that allows for intuitive scheduling of tasks, as well as some basic functional constructs to be scheduled in a parallelized manner.

# Contributions

Please follow the guidelines at CONTRIBUTING.md

Before submitting a patch, please make sure to run the test suite to avoid regressions. The `tests` binary is automatically built as part of the CMake build. Add tests liberally.
