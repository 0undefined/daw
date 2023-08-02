#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>

#include <engine/types.h>
#include <engine/logging.h>
#include <engine/stack.h>
#include <engine/vector.h>
#include <engine/memory.h>
#include <engine/input.h>
#include <engine/state.h>

typedef struct {
  u32 texture_id;
  i32 x, y,
      w, h;
} RenderUnit;

typedef struct {
  const char *font_path;
  i32 ptsize;
} FontSpec;

typedef struct {
  i32 width;
  i32 height;
  const char *path;
} TextureSpec;

typedef struct Window Window;

#define NUM_GLOBAL_BINDINGS 1
typedef struct {
  void    *data; /* Contains textures and such */
  u64 data_len;

  Window *window;
  bool    quit;

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
  char  *edit_text;
  usize edit_pos;

  memory *mem;

  i_ctx **bindings;
  usize bindings_sz;
  usize bindings_len;

  binding_t bindings_global[NUM_GLOBAL_BINDINGS];
} Platform;

/* Essential functions */
Platform *engine_init(
    const char        *windowtitle,
    v2_i32             windowsize,
    const f32          render_scale,
    const u32          flags,
    const usize       initial_memory,
    const FontSpec    *fonts[],
    const TextureSpec *textures[]);

i32 engine_run(Platform *p, StateType initial_state);

void engine_stop(Platform *p);

/* Utility functions */
void engine_fps_max(u64 cap);
void engine_input_ctx_push(i_ctx *ctx);
void engine_input_ctx_pop(void);
void engine_input_ctx_reset(void);

void render_set_zoom(f32 new_zoom);
void render_adjust_zoom(f32 diff);
void render_add_unit(RenderUnit *u);

u32 get_time(void);
v2_i32   get_windowsize(void);
v2_i32  *get_mousepos(void);

#include "rendering.h"

#ifdef ENGINE_INTERNALS

#define MAX(a,b) (a > b ? a : b)

/* Window */
struct Window {
  SDL_Window    *window;
  SDL_Renderer  *renderer;
  f32          render_scale;

  v2_i32 windowsize;

  i32* game_w;
  i32* game_h;
};

typedef struct {
  SDL_Texture *texture;
  const i32 tilesize;
  const i32 width;
  const i32 height;
} Texture;

struct Resources {
	usize textures_len;
	usize textures_size;
	usize fonts_len;

	usize texturepaths_len;
	usize fontpaths_len;

	/* Paths for our sources, kept in case the user wants to reload them */
	TextureSpec **texture_paths;
	FontSpec **font_paths;

	/* Our actual sources */
	Texture **textures;
	TTF_Font **fonts;
};


#endif
#endif
