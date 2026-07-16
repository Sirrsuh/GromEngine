#pragma once

#include <Jobs/JobSystem.h>

namespace grom
{

template<typename Func>
struct ParallelForJob
{
    Func F;
    i32  Start;
    i32  End;
    i32  Stride;
};

}
