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
  due to 'up pointer' fixing starting to occur (`begin()`/`end()` starts returning nodes on the 'slow' side)

## Can this be improved? Probably.
* Removing the `if` hacks would be a good start.
* Focusing on how to prevent the current limitation of erasure degrading to O(n) would be
  a priority as well.
* A .natvis to help visualize and debug the structure

## How to build

1. Install Boost and ensure it is added to path
2. Run `configure_TARGET.sh`
3. Either run the respective `build_TARGET.sh` or use visual studio

Note: Currently this project is only tested on `Windows 11` with `vs2022` & `ninja`+`clang`

## Benchmark results

MSVC-Release:
```
push_front_profile<int>:
(n = 5000) push_front_profile_profile<deque<int>>(...): 149500
(n = 5000) push_front_profile_profile<stable_deque<int>>(...): 522200
(n = 5000) push_front_profile_profile<stable_vector<int>>(...): 20791500
(n = 5000) push_front_profile_profile<vector<int>>(...): 885400
----[push_front_profile]----
        deque<int> |  (149500)
 stable_deque<int> | ## (522200)
stable_vector<int> | #################################################################################################### (20791500)
       vector<int> | #### (885400)
-----push_front_profile-----



push_front_profile<struct BigData>:
(n = 5000) push_front_profile_profile<deque<struct BigData>>(...): 4266100
(n = 5000) push_front_profile_profile<stable_deque<struct BigData>>(...): 5094200
(n = 5000) push_front_profile_profile<stable_vector<struct BigData>>(...): 40202700
(n = 5000) push_front_profile_profile<vector<struct BigData>>(...): 1302112400
----[push_front_profile]----
        deque<struct BigData> |  (4266100)
 stable_deque<struct BigData> |  (5094200)
stable_vector<struct BigData> | ### (40202700)
       vector<struct BigData> | #################################################################################################### (1302112400)
-----push_front_profile-----



push_back_profile<int>:
(n = 5000) push_back_profile_profile<deque<int>>(...): 111300
(n = 5000) push_back_profile_profile<stable_deque<int>>(...): 639300
(n = 5000) push_back_profile_profile<stable_vector<int>>(...): 498900
(n = 5000) push_back_profile_profile<vector<int>>(...): 54200
----[push_back_profile]----
        deque<int> | ################# (111300)
 stable_deque<int> | #################################################################################################### (639300)
stable_vector<int> | ############################################################################## (498900)
       vector<int> | ######## (54200)
-----push_back_profile-----



push_back_profile<struct BigData>:
(n = 5000) push_back_profile_profile<deque<struct BigData>>(...): 4464700
(n = 5000) push_back_profile_profile<stable_deque<struct BigData>>(...): 4774900
(n = 5000) push_back_profile_profile<stable_vector<struct BigData>>(...): 5436600
(n = 5000) push_back_profile_profile<vector<struct BigData>>(...): 15351200
----[push_back_profile]----
        deque<struct BigData> | ############################# (4464700)
 stable_deque<struct BigData> | ############################### (4774900)
stable_vector<struct BigData> | ################################### (5436600)
       vector<struct BigData> | #################################################################################################### (15351200)
-----push_back_profile-----



erase_front_profile<int>:
(n = 5000) erase_front_profile_profile<deque<int>>(...): 35500
(n = 5000) erase_front_profile_profile<stable_deque<int>>(...): 144200
(n = 5000) erase_front_profile_profile<stable_vector<int>>(...): 63678700
(n = 5000) erase_front_profile_profile<vector<int>>(...): 19853400
----[erase_front_profile]----
        deque<int> |  (35500)
 stable_deque<int> |  (144200)
stable_vector<int> | #################################################################################################### (63678700)
       vector<int> | ############################### (19853400)
-----erase_front_profile-----



erase_front_profile<struct BigData>:
(n = 5000) erase_front_profile_profile<deque<struct BigData>>(...): 41000
(n = 5000) erase_front_profile_profile<stable_deque<struct BigData>>(...): 1647000
(n = 5000) erase_front_profile_profile<stable_vector<struct BigData>>(...): 75686400
(n = 5000) erase_front_profile_profile<vector<struct BigData>>(...): 2154296800
----[erase_front_profile]----
        deque<struct BigData> |  (41000)
 stable_deque<struct BigData> |  (1647000)
stable_vector<struct BigData> | ### (75686400)
       vector<struct BigData> | #################################################################################################### (2154296800)
-----erase_front_profile-----



erase_back_profile<int>:
(n = 5000) erase_back_profile_profile<stable_deque<int>>(...): 227600
(n = 5000) erase_back_profile_profile<stable_vector<int>>(...): 91700
(n = 5000) erase_back_profile_profile<vector<int>>(...): 0
----[erase_back_profile]----
        deque<int> |  (0)
 stable_deque<int> | #################################################################################################### (227600)
stable_vector<int> | ######################################## (91700)
       vector<int> |  (0)
-----erase_back_profile-----



erase_back_profile<struct BigData>:
(n = 5000) erase_back_profile_profile<stable_deque<struct BigData>>(...): 1884900
(n = 5000) erase_back_profile_profile<stable_vector<struct BigData>>(...): 194200
(n = 5000) erase_back_profile_profile<vector<struct BigData>>(...): 0
----[erase_back_profile]----
        deque<struct BigData> |  (0)
 stable_deque<struct BigData> | #################################################################################################### (1884900)
stable_vector<struct BigData> | ########## (194200)
       vector<struct BigData> |  (0)
-----erase_back_profile-----
```
