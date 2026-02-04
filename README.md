# `stable_deque`

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

## How to build

1. Run `configure.sh`
2. Install Boost and ensure it is added to path 

Note: Currently this project is only tested on `Windows 11` with `vs2022`

## Benchmark results

```
push_front_profile<int>:
(n = 5000) push_front_profile_profile<deque<int>>(...): 7743400
(n = 5000) push_front_profile_profile<stable_deque<int>>(...): 11019500
(n = 5000) push_front_profile_profile<stable_vector<int>>(...): 180229300
(n = 5000) push_front_profile_profile<vector<int>>(...): 3813200
----[push_front_profile]----
        deque<int> | #### (7743400)
 stable_deque<int> | ###### (11019500)
stable_vector<int> | #################################################################################################### (180229300)
       vector<int> | ## (3813200)
-----push_front_profile-----



push_front_profile<struct BigData>:
(n = 5000) push_front_profile_profile<deque<struct BigData>>(...): 13231500
(n = 5000) push_front_profile_profile<stable_deque<struct BigData>>(...): 15254900
(n = 5000) push_front_profile_profile<stable_vector<struct BigData>>(...): 186396100
(n = 5000) push_front_profile_profile<vector<struct BigData>>(...): 1293807700
----[push_front_profile]----
        deque<struct BigData> | # (13231500)
 stable_deque<struct BigData> | # (15254900)
stable_vector<struct BigData> | ############## (186396100)
       vector<struct BigData> | #################################################################################################### (1293807700)
-----push_front_profile-----



push_back_profile<int>:
(n = 5000) push_back_profile_profile<deque<int>>(...): 774300
(n = 5000) push_back_profile_profile<stable_deque<int>>(...): 13265500
(n = 5000) push_back_profile_profile<stable_vector<int>>(...): 13345800
(n = 5000) push_back_profile_profile<vector<int>>(...): 524500
----[push_back_profile]----
        deque<int> | ##### (774300)
 stable_deque<int> | ################################################################################################### (13265500)
stable_vector<int> | #################################################################################################### (13345800)
       vector<int> | ### (524500)
-----push_back_profile-----



push_back_profile<struct BigData>:
(n = 5000) push_back_profile_profile<deque<struct BigData>>(...): 6234100
(n = 5000) push_back_profile_profile<stable_deque<struct BigData>>(...): 18106900
(n = 5000) push_back_profile_profile<stable_vector<struct BigData>>(...): 17707200
(n = 5000) push_back_profile_profile<vector<struct BigData>>(...): 16700700
----[push_back_profile]----
        deque<struct BigData> | ################################## (6234100)
 stable_deque<struct BigData> | #################################################################################################### (18106900)
stable_vector<struct BigData> | ################################################################################################# (17707200)
       vector<struct BigData> | ############################################################################################ (16700700)
-----push_back_profile-----



erase_front_profile<int>:
(n = 5000) erase_front_profile_profile<deque<int>>(...): 12110800
(n = 5000) erase_front_profile_profile<stable_deque<int>>(...): 15018700
(n = 5000) erase_front_profile_profile<stable_vector<int>>(...): 316584600
(n = 5000) erase_front_profile_profile<vector<int>>(...): 21757000
----[erase_front_profile]----
        deque<int> | ### (12110800)
 stable_deque<int> | #### (15018700)
stable_vector<int> | #################################################################################################### (316584600)
       vector<int> | ###### (21757000)
-----erase_front_profile-----



erase_front_profile<struct BigData>:
(n = 5000) erase_front_profile_profile<deque<struct BigData>>(...): 11919500
(n = 5000) erase_front_profile_profile<stable_deque<struct BigData>>(...): 16366500
(n = 5000) erase_front_profile_profile<stable_vector<struct BigData>>(...): 347976400
(n = 5000) erase_front_profile_profile<vector<struct BigData>>(...): 1996307700
----[erase_front_profile]----
        deque<struct BigData> |  (11919500)
 stable_deque<struct BigData> |  (16366500)
stable_vector<struct BigData> | ################# (347976400)
       vector<struct BigData> | #################################################################################################### (1996307700)
-----erase_front_profile-----



erase_back_profile<int>:
(n = 5000) erase_back_profile_profile<stable_deque<int>>(...): 18587000
(n = 5000) erase_back_profile_profile<stable_vector<int>>(...): 114631100
(n = 5000) erase_back_profile_profile<vector<int>>(...): 2720700
----[erase_back_profile]----
        deque<int> |  (0)
 stable_deque<int> | ################ (18587000)
stable_vector<int> | #################################################################################################### (114631100)
       vector<int> | ## (2720700)
-----erase_back_profile-----



erase_back_profile<struct BigData>:
(n = 5000) erase_back_profile_profile<stable_deque<struct BigData>>(...): 20751200
(n = 5000) erase_back_profile_profile<stable_vector<struct BigData>>(...): 146039800
(n = 5000) erase_back_profile_profile<vector<struct BigData>>(...): 2576800
----[erase_back_profile]----
        deque<struct BigData> |  (0)
 stable_deque<struct BigData> | ############## (20751200)
stable_vector<struct BigData> | #################################################################################################### (146039800)
       vector<struct BigData> | # (2576800)
-----erase_back_profile-----
```