#include <Core/Assert.h>
#include <cstdio>

namespace grom
{

void GromAssert(const char* expr, const char* file, i32 line, const char* msg)
{
    if (msg)
    {
        fprintf(stderr, "ASSERTION FAILED: %s\n  Expression: %s\n  File: %s\n  Line: %d\n", msg, expr, file, line);
    }
    else
    {
        fprintf(stderr, "ASSERTION FAILED: %s\n  File: %s\n  Line: %d\n", expr, file, line);
    }
    GROM_BREAK;
}

}
