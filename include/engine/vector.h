#ifndef VECTOR_H
#define VECTOR_H

#include "types.h"

#include <stdbool.h>
#include <stdio.h>

typedef struct {
  i32 x;
  i32 y;
} v2_i32;

bool v2_i32_eq(const v2_i32 a, const v2_i32 b);

v2_i32 v2_i32_add(v2_i32 a, v2_i32 b);
v2_i32 v2_i32_add_i(v2_i32 a, i32 b);
v2_i32 v2_i32_sub(v2_i32 a, v2_i32 b);
v2_i32 v2_i32_sub_i(v2_i32 a, i32 b);
v2_i32 v2_i32_div(v2_i32 a, v2_i32 b);
v2_i32 v2_i32_div_i(v2_i32 a, i32 b);
v2_i32 v2_i32_mul(v2_i32 a, v2_i32 b);
v2_i32 v2_i32_mul_i(v2_i32 a, i32 b);
v2_i32 v2_i32_mod(v2_i32 a, v2_i32 b);
v2_i32 v2_i32_mod_i(v2_i32 a, i32 b);
v2_i32 v2_i32_max(v2_i32 a, v2_i32 b);
v2_i32 v2_i32_min(v2_i32 a, v2_i32 b);
v2_i32 v2_i32_lerp(f32 dt, v2_i32 a, v2_i32 b);

void v2_i32_fprintf(FILE* stream, v2_i32 a);
#endif
