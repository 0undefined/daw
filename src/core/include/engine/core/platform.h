#ifndef ENGINE_CORE_PLATFORM_H
#define ENGINE_CORE_PLATFORM_H

#include <engine/core/types.h>
#include <engine/core/memory.h>
#include <engine/ctrl/input.h>
#include <engine/rendering/window.h>
#include <engine/utils/vector.h>

#define NUM_GLOBAL_BINDINGS 1

/* Defines the internally used state of the engine.
 * A single instance is created during `engine_init`, and used as a global
 * variable (yeah, im sorry). Due to this design flaw the engine as a whole is
 * not quite thread safe.
 */
typedef struct Platform {
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

  /* The ctrl is probably the only sensible thing in this struct. */
  i_ctx** bindings;
  usize bindings_sz;
  usize bindings_len;

  binding_t bindings_global[NUM_GLOBAL_BINDINGS];
} Platform;

#endif
