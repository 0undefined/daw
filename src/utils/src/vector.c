#include <engine/utils.h>
#include <engine/vector.h>

bool v2_i32_eq(const v2_i32 a, const v2_i32 b) {
  return (a.x == b.x) && (a.y == b.y);
}
v2_i32 v2_i32_add(v2_i32 a, v2_i32 b) { return (v2_i32){a.x + b.x, a.y + b.y}; }
v2_i32 v2_i32_add_i(v2_i32 a, i32 b) { return (v2_i32){a.x + b, a.y + b}; }
v2_i32 v2_i32_sub(v2_i32 a, v2_i32 b) { return (v2_i32){a.x - b.x, a.y - b.y}; }
v2_i32 v2_i32_sub_i(v2_i32 a, i32 b) { return (v2_i32){a.x - b, a.y - b}; }
v2_i32 v2_i32_div(v2_i32 a, v2_i32 b) { return (v2_i32){a.x / b.x, a.y / b.y}; }
v2_i32 v2_i32_div_i(v2_i32 a, i32 b) { return (v2_i32){a.x / b, a.y / b}; }
v2_i32 v2_i32_mul(v2_i32 a, v2_i32 b) { return (v2_i32){a.x * b.x, a.y * b.y}; }
v2_i32 v2_i32_mul_i(v2_i32 a, i32 b) { return (v2_i32){a.x * b, a.y * b}; }
v2_i32 v2_i32_mod(v2_i32 a, v2_i32 b) { return (v2_i32){a.x % b.x, a.y % b.y}; }
v2_i32 v2_i32_mod_i(v2_i32 a, i32 b) { return (v2_i32){a.x % b, a.y % b}; }
v2_i32 v2_i32_max(v2_i32 a, v2_i32 b) {
  return (v2_i32){MAX(a.x, b.x), MAX(a.y, b.y)};
}
v2_i32 v2_i32_min(v2_i32 a, v2_i32 b) {
  return (v2_i32){MIN(a.x, b.x), MIN(a.y, b.y)};
}
v2_i32 v2_i32_lerp(f32 dt, v2_i32 a, v2_i32 b) {
  return (v2_i32){
      .x = lerp(dt, (f32)a.x, (f32)b.x),
      .y = lerp(dt, (f32)a.y, (f32)b.y),
  };
}

void v2_i32_fprintf(FILE* stream, v2_i32 a) {
  fprintf(stream, "<%d,%d>", a.x, a.y);
}
