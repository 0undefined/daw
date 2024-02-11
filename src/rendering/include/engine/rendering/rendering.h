#ifndef ENGINE_RENDERING_RENDERING_H
#define ENGINE_RENDERING_RENDERING_H

#include <engine/core/types.h>
#include <engine/utils/vector.h>
#include <engine/rendering/window.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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

typedef struct {
  /* Shader proram */
  u32 program;
} Shader;

typedef struct {
  /* Shader proram */
  Shader shader;
  /* Vertex Array Object */
  u32 vao;
  /* Vertex Buffer Object */
  u32 vbo;
  /* Index Buffer Object */
  u32 ibo;
  /* Color (?) */
  u32 col;
  /* MVP (a uniform from the shader) */
  u32 mvp;
} RenderObject;

/* Rendering functions */
void render_begin(Window w);
void render_present(Window w);
void drawcall_reset(void);
void render(Window w);

/* Misc */
//void window_size_callback(GLFWwindow* window, i32 width, i32 height);
//void engine_window_resize_pointers(i32* w, i32* h);
//void engine_window_resize_pointers_reset(void);

void engine_draw_sprite(Sprite* s, v2_i32* pos, f32 scale);
void engine_draw_sprite_ex(Sprite* s, v2_i32* pos, f32 scale,
                           Engine_color colormod);

Sprite sprite_new(u64 tid, u8 coord);

typedef enum {
  RenderDrawCallType_Text,
  RenderDrawCallType_Sprite,
} RenderDrawCallType;
//
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

RenderObject RenderObject_new();

#endif
