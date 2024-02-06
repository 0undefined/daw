#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>

// TODO: Cleanup the includes, ideally this header file should only include all
// "public-facing" headers.

#include <engine/core/types.h>
#include <engine/core/logging.h>
#include <engine/core/memory.h>
#include <engine/core/state.h>
#include <engine/ctrl/input.h>
#include <engine/resources.h>
#include <engine/utils/stack.h>
#include <engine/utils/vector.h>

typedef struct {
  u32 texture_id;
  i32 x, y, w, h;
} RenderUnit;

#include <engine/rendering/window.h>

#define NUM_GLOBAL_BINDINGS 1
typedef struct {
  void* data; /* Contains textures and such */
  u64 data_len;

  Window window;
  bool quit;

  u64 frame;
  f32 fps_target;

  /* TODO: Move mouse data to input ctx */
  v2_i32 mouse_pos;

  v2_i32 mousedown;
  v2_i32 mouseup;

  bool mouse_lclick;
  bool mouse_rclick;

  i32 camera_x;
  i32 camera_y;

  /* Text input/editing is currently not used/implemented */
  char* edit_text;
  usize edit_pos;

  memory* mem;

  i_ctx** bindings;
  usize bindings_sz;
  usize bindings_len;

  struct RenderObject *testobject;

  binding_t bindings_global[NUM_GLOBAL_BINDINGS];
} Platform;

/* Essential functions */
Platform* engine_init(const char* windowtitle, i32 windowWidth, i32 windowHeight,
                      const f32 render_scale, const u32 flags,
                      const usize initial_memory, const Asset_FontSpec* fonts[],
                      const Asset_TextureSpec* textures[]);

i32 engine_run(Platform* p, StateType initial_state);

void engine_stop(Platform* p);

/* Utility functions */
void engine_fps_max(u64 cap);

void render_set_zoom(f32 new_zoom);
void render_adjust_zoom(f32 diff);
void render_add_unit(RenderUnit* u);

f64 get_time(void);
//v2_i32 get_windowsize(void);
v2_i32* get_mousepos(void);

/* Input handling */
void engine_input_ctx_push(i_ctx* ctx);
void engine_input_ctx_pop(void);
void engine_input_ctx_reset(void);

//#include <engine/rendering/rendering.h>

//#ifdef ENGINE_INTERNALS
//
//#include <glad/gl.h>
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>
//
//
//#endif
#endif
