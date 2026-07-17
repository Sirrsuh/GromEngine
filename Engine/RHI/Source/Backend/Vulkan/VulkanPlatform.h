#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <vulkan/vulkan.h>

namespace grom::Platform {

inline void* LoadLibrary(const char* name)
{
    return ::LoadLibraryA(name);
}

inline void FreeLibrary(void* lib)
{
    ::FreeLibrary(static_cast<HMODULE>(lib));
}

inline void* GetProcAddress(void* lib, const char* name)
{
    return ::GetProcAddress(static_cast<HMODULE>(lib), name);
}

} // namespace grom::Platform
