#pragma once

#include <Core/Types.h>
#include <Core/Container.h>

namespace grom
{

enum class EPlatformType
{
    Windows,
    Linux,
    Mac
};

EPlatformType GetPlatformType();
GString GetPlatformName();
f64 GetTimeSeconds();
f64 GetTimeMilliseconds();
void SleepMs(u32 ms);

struct PlatformInfo
{
    GString OSName;
    GString CPUName;
    u64 TotalRAM = 0;
    u32 NumCores = 0;
    u32 NumLogicalCPUs = 0;
};

PlatformInfo QueryPlatformInfo();

} // namespace grom
