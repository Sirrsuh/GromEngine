#pragma once

#include <cstdint>
#include <cstddef>

namespace grom
{

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using usize = size_t;
using uint = u32;

#define GROM_FORCEINLINE __forceinline
#define GROM_ALIGN(x) __declspec(align(x))
#define GROM_ASSUME(x) __assume(x)
#define GROM_BREAK __debugbreak()
#define GROM_UNUSED(x) (void)(x)
#define GROM_BIT(x) (1u << (x))

constexpr f32 GROM_PI = 3.14159265358979323846f;
constexpr f32 GROM_HALF_PI = GROM_PI / 2.0f;

}
