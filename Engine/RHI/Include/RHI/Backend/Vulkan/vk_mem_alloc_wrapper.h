#pragma once

// Vulkan Memory Allocator wrapper - suppresses warnings from VMA headers

#pragma warning(push, 0)
#define VMA_NULL VK_NULL_HANDLE
#define VMA_LEAK_LOG_FORMAT(...)
#define VMA_ASSERT(expr)
#define VMA_DEBUG_LOG(format, ...) 

// Suppress C4100 (unreferenced parameter) and other warnings from VMA
#pragma warning(disable: 4100)  // unreferenced formal parameter
#pragma warning(disable: 4189)  // local variable initialized but not referenced
#pragma warning(disable: 4189)  // local variable initialized but not referenced
#pragma warning(disable: 4127)  // conditional expression is constant
#pragma warning(disable: 4200)  // nonstandard extension used: zero-sized array
#pragma warning(disable: 4201)  // nonstandard extension used: nameless struct/union
#pragma warning(disable: 4244)  // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4324)  // structure was padded due to alignment specifier
#pragma warning(disable: 4365)  // signed/unsigned mismatch
#pragma warning(disable: 4505)  // unreferenced local function has been removed
#pragma warning(disable: 4668)  // 'symbol' is not defined as a preprocessor macro
#pragma warning(disable: 4820)  // padding added after data member

#include <vk_mem_alloc.h>

#pragma warning(pop)