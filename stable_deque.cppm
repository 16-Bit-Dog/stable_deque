// C++20 Module interface for stable_deque
// This module wraps the header-only implementation for module-based usage

module;

// Global module fragment - include all headers before the module declaration
#include <cstdint>
#include <deque>
#include <memory>
#include <cassert>

#if defined(_WIN32)
	#include <malloc.h>
#else
	#include <alloca.h>
#endif

export module stable_deque;

// Include the implementation in the module purview
#include "stable_deque.h"

// Export the stable_deque class template
export using ::stable_deque;
