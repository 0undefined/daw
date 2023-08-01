#include <stdlib.h>
#include <stdint.h>

#include <string.h>

#include <engine/utils.h>
#include <engine/logging.h>

f32 lerp(f32 dt, f32 a, f32 b) {
	return (a * (1.0f - dt)) + (b * dt);
}

i32 int_lerp(f32 dt, i32 a, i32 b) {
	return ((f32)a * (1.0f - dt)) + ((f32)b * dt);
}

u32 hash(char *str) {
	u32 sum = 0;
	while (*str != '\0') {
		sum ^= (*str) * 0xdeece66d + 0xb;
		str++;
	}
	return sum;
}

/* Populates dstmap
 * on success: return pointer to dstmap
 * on failure: return NULL */
i32* kernmap(const void *map, i32 *dstmap, const v2_i32 mapsize, predicate_t *predicate) {
	const i32 w = mapsize.x;
	const i32 h = mapsize.y;
	i32 mask[w*h];

	if (w * h < 1) return NULL;

	for (i32 i = 0; i < w * h; i++) {
		mask[i] = predicate((void*)((u64)map + sizeof(i32) * i)) ? 1 : 0;
	}

	for (i32 y = 1; y < h - 1; y++) {
		for (i32 x = 1; x < w - 1; x++) {
			const i32 global_idx = (y * w) + x;
			const i32 offs = global_idx - w - 1;
			i32 _sum = 0;

			i32 shift = 0;

			/* We go in the following order */
			/* ....|0|1|2|....*/
			/* ....|3|4|5|....*/
			/* ....|6|7|8|....*/
			/* Where `4` is in the center, MASK_C */
			for (i32 yy = offs; yy <= offs + w + w; yy += w) {
				for (i32 xx = yy; xx < yy + 3; xx++) {
					_sum = _sum | (mask[xx] << shift++);
				}
			}

			dstmap[global_idx] = _sum;
		}
	}
	return dstmap;
}

/* Returns an index from the given weights. */
i32 pick_from_sample(const i32 *weights, i32 len) {
	if (len <= 0) return 0;

  /* Cumulative sum */
  i32 cumweights[len];
  i32 sum = 0;
  for (i32 i = 0; i < len; i++) {
    sum += weights[i];
    cumweights[i] = sum;
  }

	if (sum == 0) return 0;

  i32 pick = rand() % sum;

  for (i32 i = 0; i < len; i++) {
    if (pick < cumweights[i]) return i;
  }
  return -1;
}
