cmake_minimum_required(VERSION 3.28)

project(stable_deque LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Option to build with C++20 modules support
option(STABLE_DEQUE_USE_MODULES "Build with C++20 modules support" OFF)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Boost REQUIRED)

enable_testing()

# Header-only library (traditional usage)
add_library(stable_deque_header INTERFACE)
target_include_directories(stable_deque_header INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# C++20 Module library (when modules are enabled)
if(STABLE_DEQUE_USE_MODULES)
  # Check if the compiler supports modules
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14")
    message(WARNING "GCC ${CMAKE_CXX_COMPILER_VERSION} may have limited C++20 modules support. GCC 14+ recommended.")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16")
    message(WARNING "Clang ${CMAKE_CXX_COMPILER_VERSION} may have limited C++20 modules support. Clang 16+ recommended.")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND MSVC_VERSION VERSION_LESS "1930")
    message(WARNING "MSVC ${MSVC_VERSION} may have limited C++20 modules support. VS2022+ recommended.")
  endif()

  add_library(stable_deque_module)
  target_sources(stable_deque_module
    PUBLIC
      FILE_SET CXX_MODULES FILES
        stable_deque.cppm
  )
  target_compile_features(stable_deque_module PUBLIC cxx_std_20)
  target_include_directories(stable_deque_module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Test executable
add_executable(deque_tests deque_tests.cpp)
target_link_libraries(deque_tests PRIVATE GTest::gtest_main)
target_include_directories(deque_tests PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

# Copy test data files to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/magic_data.txt ${CMAKE_CURRENT_BINARY_DIR}/magic_data.txt COPYONLY)

include(GoogleTest)
gtest_discover_tests(deque_tests)

# Example target using modules (only when modules are enabled)
if(STABLE_DEQUE_USE_MODULES)
  add_executable(module_example_target EXCLUDE_FROM_ALL)
  target_sources(module_example_target PRIVATE
    $<$<BOOL:${STABLE_DEQUE_USE_MODULES}>:${CMAKE_CURRENT_SOURCE_DIR}/examples/module_example.cpp>
  )
  target_link_libraries(module_example_target PRIVATE stable_deque_module)
  target_compile_features(module_example_target PUBLIC cxx_std_20)
endif()
