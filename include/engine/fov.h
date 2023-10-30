#ifndef ENGINE_FOV_H
#define ENGINE_FOV_H

#include "types.h"
#include "vector.h"
#include <stdbool.h>

/* `fov_shadowcast`: */
/*   map:         your 2D enum tile grid
 *   mapsize:     x: width, y: height of the map
 *   visblocking: pointer to a function that returns `true` when receiving a
 *                pointer to a LOS blocking tile
 *   lightmap:    [out] 2D lightmap, this is simply overwritten with the
 * distance to the source. range:       visibility range/radius. src:         2D
 * point to calculate FOV from
 *   */
void fov_shadowcast(const void* map, const v2_i32 mapsize,
                    bool (*visblocking)(const void*), i32* lightmap,
                    const i32 range, v2_i32 src);

#endif
