# `stable_deque`

[![CI](https://github.com/16-Bit-Dog/stable_deque/actions/workflows/ci.yml/badge.svg)](https://github.com/16-Bit-Dog/stable_deque/actions/workflows/ci.yml)

A C++20 deque implementation with stable iterators/references.

## What a deque is needed for:
- Fast front/back erase/insert
- O(1) random access
- Middle insert/erase can be slow, doesn't really
  matter since we primarily use a deque as a double
  sided queue.

## Does this container address these requirments
- [x] Fast front/back erase/insert
- [x] O(1) random access

## What this implementation does that is special:
* The core of the structure is like a `boost::stable_vector`
  (indirection stores each node location). The inner data
  is stable due to use of 'up pointers'
* The backing data structure is a `deque` instead of a `vector`
* Instead of fixing 'up pointers' (refer to `stable_vector`'s
  implementation), we assign a `middle` to our `deque` to split
  the structure into two halves (a left and right). This enables
  us to avoid requiring to do an expensive "up pointer' fix pass 
  every `push_front`/`erase`.

## Requirements

- C++20 compatible compiler:
  - GCC 13+ (GCC 14+ for modules)
  - Clang 16+
  - MSVC 2022+
- CMake 3.28+
- Boost (for tests only)

## Building

### Header-Only (Traditional)

```bash
mkdir build && cd build
cmake ..
cmake --build .
ctest
```

### With C++20 Modules

```bash
mkdir build && cd build
cmake .. -DSTABLE_DEQUE_USE_MODULES=ON
cmake --build .
```

**Note:** C++20 modules support varies by compiler. MSVC and Clang 16+ have the best support.

## Usage

### Header-Only (Traditional)

```cpp
#include "stable_deque.h"

int main() {
    stable_deque<int> sd;
    sd.push_back(1);
    sd.push_front(0);
    
    auto it = sd.begin();
    sd.push_back(2);  // Iterator 'it' remains valid!
    
    return *it;  // Returns 0
}
```

### C++20 Modules

```cpp
import stable_deque;

int main() {
    stable_deque<int> sd;
    sd.push_back(1);
    sd.push_front(0);
    
    auto it = sd.begin();
    sd.push_back(2);  // Iterator 'it' remains valid!
    
    return *it;  // Returns 0
}
```

### CMake Integration

#### Header-Only

```cmake
add_subdirectory(stable_deque)
target_link_libraries(your_target PRIVATE stable_deque_header)
```

#### Modules

```cmake
add_subdirectory(stable_deque)
target_link_libraries(your_target PRIVATE stable_deque_module)
```

## Limitations
* Inserting/erasing in the middle of the deque is O(n) due to 'up pointer' fixing
  (but this is expected for a `stable_deque`/`stable_vector`)
* If erasing more than a side pushes, performance degrades from O(1) to O(n)
  due to 'up pointer' fixing starting to occur (`begin()` starts returning nodes on the right-hand-side)

## Can this be improved? Probably.
* Removing the `if` hacks would be a good start.
* Focusing on how to prevent the current limitation of erasure degrading to O(n) would be
  a priority as well.
* A .natvis to help visualize and debug the structure