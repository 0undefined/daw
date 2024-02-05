#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* Signed */
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/* Unsigned */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

/* floating points */
typedef float f32;
typedef double f64;

/* sizes */
#if __x86_64__ || __ppc64__ || _WIN64
typedef u64 usize;
typedef i64 isize;
#else
typedef u32 usize;
typedef i32 isize;
#endif

typedef bool(predicate_t)(const void*);

#endif
