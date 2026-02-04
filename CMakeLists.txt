cmake_minimum_required(VERSION 3.10)

project(stable_deque)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  deque_tests
  deque_tests.cpp
)
target_link_libraries(
  deque_tests
  GTest::gtest_main
)
target_include_directories(
  deque_tests
  PRIVATE
  ${Boost_INCLUDE_DIRS}
)
include(GoogleTest)
gtest_discover_tests(deque_tests)
