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