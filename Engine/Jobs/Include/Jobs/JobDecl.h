#pragma once

#include <Core/Types.h>
#include <Core/Container.h>

namespace grom
{

using JobFunc = void(*)(void*);

struct JobDecl
{
    JobFunc  Function;
    void*    Data;
    u32      Size;
    GString  Name;
};

enum class EJobPriority : u32
{
    Low      = 0,
    Normal   = 1,
    High     = 2,
    Critical = 3
};

using JobHandle = u64;

}
