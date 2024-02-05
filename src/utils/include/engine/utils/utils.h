#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include "types.h"
#include "vector.h"

#define MIN(a, b) ((a < b) ? (a) : (b))
#define MAX(a, b) ((a > b) ? (a) : (b))

#define MASK_TL (1 << 0)
#define MASK_T (1 << 1)
#define MASK_TR (1 << 2)
#define MASK_L (1 << 3)
#define MASK_C (1 << 4)
#define MASK_R (1 << 5)
#define MASK_BL (1 << 6)
#define MASK_B (1 << 7)
#define MASK_BR (1 << 8)

/* Linear interpolate */
f32 lerp(f32 dt, f32 a, f32 b);
i32 int_lerp(f32 dt, i32 a, i32 b);

/* Hashes */
u32 hash(char* str);

/* Masks surrounding tiles of a kernel size of 3x3 */
/* In reality we only need 9 bits for this, but I think I had a reason for using
 * i32 */
i32* kernmap(const void* map, i32* dstmap, const v2_i32 mapsize,
             predicate_t* predicate);

/* Returns an index from the given weights. */
i32 pick_from_sample(const i32* weights, i32 len);

#endif
