#include <engine/utils/fov.h>
#include <engine/utils.h>
#include <math.h>
#include <stdint.h>

void fov_shadowcast_rec(const void* map, const v2_i32 mapsize,
                        bool (*visblocking)(const void*), i32* lightmap,
                        const i32 range, v2_i32 src, const i32 row, f32 start,
                        const f32 end, const i8 xx, const i8 xy, const i8 yx,
                        const i8 yy) {

  if (start < end) return;

  const i32 range_2 = range * range;
  f32 new_start = start;

  for (i32 i = row; i <= range; i++) {
    i32 dx = (-1 * i) - 1;
    i32 dy = -1 * i;

    bool blocked = false;

    while (dx <= 0) {
      dx += 1;

      const i32 mapx = src.x + dx * xx + dy * xy;
      const i32 mapy = src.y + dx * yx + dy * yy;

      const f32 slope_l = (((f32)dx) - 0.5f) / (((f32)dy) + 0.5f);
      const f32 slope_r = (((f32)dx) + 0.5f) / (((f32)dy) - 0.5f);

      if (start < slope_r) continue;
      if (end > slope_l) break;

      if (dx * dx + dy * dy < range_2) {
        /* set as visible */
        if (mapx >= 0 && mapx < (long)mapsize.x && mapy >= 0 &&
            mapy < (long)mapsize.y) {
          // TODO: Calculate proper dist from source
          f32 x_2 = (src.x - mapx) * (src.x - mapx);
          f32 y_2 = (src.y - mapy) * (src.y - mapy);
          lightmap[mapy * mapsize.x + mapx] =
              MAX(lightmap[mapy * mapsize.x + mapx],
                  range - sqrt((f32)(x_2 + y_2)));
        }
      }

      /* sizeof(i32) is the size of enums */
      /* -- unless the compiler doesn't follow standard behaviour */
      const bool is_blocked = visblocking(
          (void*)((u64)map + sizeof(i32)                     /* ~ enum size */
                                 * (mapsize.x * mapy + mapx) /* index */
                  ));

      if (blocked) {
        if (!is_blocked) {
          new_start = slope_r;
        } else {
          blocked = false;
          start = new_start;
        }
      } else if (!is_blocked && i < range) {
        blocked = true;
        fov_shadowcast_rec(map, mapsize, visblocking, lightmap, range, src,
                           i + 1, start, slope_l, xx, xy, yx, yy);
        new_start = slope_r;
      }
    }

    if (blocked) break;
  }
}

/* http://www.roguebasin.com/index.php?title=FOV_using_recursive_shadowcasting
 */
void fov_shadowcast(const void* map, const v2_i32 mapsize,
                    bool (*visblocking)(const void*), i32* lightmap,
                    const i32 range, const v2_i32 src) {

  const i8 m[4][8] = {
      {1, 0, 0, -1, -1, 0, 0, 1},
      {0, 1, -1, 0, 0, -1, 1, 0},
      {0, 1, 1, 0, 0, -1, -1, 0},
      {1, 0, 0, 1, -1, 0, 0, -1},
  };

  for (i32 oct = 0; oct < 8; oct++) {
    fov_shadowcast_rec(map, mapsize, visblocking, lightmap, range, src, 1, 1.0,
                       0.0, m[0][oct], m[1][oct], m[2][oct], m[3][oct]);
  }

  /* The center is the most lit square */
  lightmap[src.y * mapsize.x + src.x] = range;
}
