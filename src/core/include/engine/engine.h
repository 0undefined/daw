#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/* TODO: Cleanup the includes, ideally this header file should only include all
 * "public-facing" headers.
 */

#include <engine/core/types.h>
#include <engine/core/logging.h>
#include <engine/core/memory.h>
#include <engine/core/state.h>
#include <engine/ctrl/input.h>
#include <engine/utils/stack.h>
#include <engine/utils/vector.h>

typedef struct {
  u32 texture_id;
  i32 x, y, w, h;
} RenderUnit;

#include <engine/rendering/window.h>
#include <engine/core/platform.h>

#include <cglm/cglm.h>

/* Essential functions */
Platform* engine_init(const char* windowtitle, i32 windowWidth, i32 windowHeight,
                      const f32 render_scale, const u32 flags,
                      const usize initial_memory);

i32 engine_run(Platform* p, StateType initial_state, void* state_arg);

void engine_stop(Platform* p);

/* Utility functions */
void engine_fps_max(u64 cap);

void render_set_zoom(f32 new_zoom);
void render_adjust_zoom(f32 diff);
void render_add_unit(RenderUnit* u);

/* move this */
void delay(uint32_t ms);

// file operations
isize f_get_sz(FILE* f);


#ifdef __cplusplus
}
#endif
#endif
