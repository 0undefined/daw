#ifndef RENDERING_H
#define RENDERING_H

#include "types.h"
#include "vector.h"

/* Definitions */
#define RGBA(_r, _g, _b, _a)                                                   \
  ((Engine_color){.r = _r, .g = _g, .b = _b, .a = _a})
#define RGB(_r, _g, _b) RGBA(_r, _g, _b, 0xFF)

/* Types */
typedef struct {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} Engine_color;

typedef struct {
  u32 texture_id;
  v2_i32 coord;
} Sprite;


#include "engine.h"
#include "ui.h"

/* Rendering functions */
void render_begin(Window* w);
void render_present(Window* w);
void drawcall_reset(void);
void render(Window* w);

/* Misc */
void engine_window_resize_pointers(i32* w, i32* h);
void engine_window_resize_pointers_reset(void);

/* UI rendering */
/* See rendering_ui.c for implementation */
i64 engine_render_text(i32 font_id, Engine_color fg, char* text,
                       v2_i32* size_out, bool wrapped);
void engine_draw_uitree(UITree* t);
void engine_draw_sprite(Sprite* s, v2_i32* pos, f32 scale);
void engine_draw_sprite_ex(Sprite* s, v2_i32* pos, f32 scale,
                           Engine_color colormod);

Sprite sprite_new(u64 tid, u8 coord);

#ifdef ENGINE_INTERNALS
#include "engine.h"

//#include <glad/gl.h>
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>

#define TEXTURES_INCREMENT 512

typedef enum {
  RenderDrawCallType_UITree,
  /*RenderDrawCallType_UIButton,*/
  RenderDrawCallType_Text,
  RenderDrawCallType_Sprite,
} RenderDrawCallType;

typedef struct {
  RenderDrawCallType type;
  union {
    void* data;
    struct {
      Sprite* sprite;
      i32 x;
      i32 y;
      f32 scale;
    } sprite;
  } data;
} RenderDrawCall;

struct RenderObject {
  u32 vao;
  u32 vbo;
  u32 shaderprogram;
  f32 g_vertex_buffer_data[9];
};

void render_uitree(Window* w, UITree* t);

void render_container(Window* w, UITree_container* t);
void render_button(Window* w, UITree_button* t);
void render_title(Window* w, UITree_title* t);
void render_text(Window* w, UITree_text* t);
v2_i32 elem_size(UITree* root);

#endif

#endif
