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
auto future = pool.schedule([] () -> int { return 42; });
...
std::cout << "The result is " << future.get << std::endl;
```

# Contributions

Please follow the guidelines at CONTRIBUTING.md

Before submitting a patch, please make sure to run the test suite to avoid regressions. The `tests` binary is automatically built as part of the CMake build. Add tests liberally.
