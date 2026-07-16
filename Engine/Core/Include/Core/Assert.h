#pragma once

#include <Core/Types.h>

namespace grom
{

void GromAssert(const char* expr, const char* file, i32 line, const char* msg);

}

#if defined(_DEBUG) || defined(DEBUG)

#define GROM_ASSERT(expr, msg) \
    do { \
        if (!(expr)) \
        { \
            grom::GromAssert(#expr, __FILE__, __LINE__, msg); \
        } \
    } while(0)

#define GROM_CHECK(expr) \
    do { \
        if (!(expr)) \
        { \
            grom::GromAssert(#expr, __FILE__, __LINE__, nullptr); \
        } \
    } while(0)

#define GROM_TODO(msg) \
    do { \
        grom::GromAssert("TODO", __FILE__, __LINE__, msg); \
    } while(0)

#else

#define GROM_ASSERT(expr, msg) GROM_ASSUME(expr)
#define GROM_CHECK(expr) GROM_ASSUME(expr)
#define GROM_TODO(msg)

#endif
