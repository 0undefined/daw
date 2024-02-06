#include <stdio.h>
#include <string.h>

//#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


#define ENGINE_INTERNALS
#include <engine/engine.h>
#include <engine/rendering/rendering.h>

/* Extern globals */
extern Platform* GLOBAL_PLATFORM;

/* Globals */
#define drawcall_limit (64 * 1024)
RenderDrawCall drawcalls[drawcall_limit];
i32 drawcall_len = 0;

//struct RenderObject;

/* Implementations */

/* Clear the screen,
 * To be used inbetween draw calls */
void render_begin(Window* w) {
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glfwMakeContextCurrent(w->window);
  w->context->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_present(Window* w) {
//  for (i32 i = 0; i < drawcall_len; i++) {
//    RenderDrawCall dc = drawcalls[i];
//    switch (dc.type) {
//    case RenderDrawCallType_UITree:
//      render_uitree(w, dc.data.data);
//      break;
//    case RenderDrawCallType_Text:
//      LOG("RenderDrawCallType_Text rendering not implemented!");
//      break;
//    case RenderDrawCallType_Sprite: {
//#ifdef _DEBUG
//      if (dc.data.sprite.sprite == NULL) {
//        __asm__("int3;");
//        WARN("Sprite %lx in drawcall %d/%d had NULL reference",
//             dc.data.sprite.sprite, i, drawcall_len);
//
//        drawcall_len = 0;
//        exit(EXIT_FAILURE);
//      }
//#endif
//      Sprite s = *dc.data.sprite.sprite;
//      Texture* t =
//          ((struct Resources*)GLOBAL_PLATFORM->data)->textures[s.texture_id];
//      i32 ts = t->tilesize;
//    } break;
//    default:
//      break;
//    }
//  }

  drawcall_len = 0;

  glfwSwapBuffers(w->window);
}

void drawcall_reset(void) { drawcall_len = 0; }

void window_size_callback(GLFWwindow* window, int width, int height) {
  GLOBAL_PLATFORM->window->context->Viewport(0,0, width, height);
  //*GLOBAL_PLATFORM->window->game_w = width;
  //*GLOBAL_PLATFORM->window->game_h = height;
  GLOBAL_PLATFORM->window->windowsize.x = width;
  GLOBAL_PLATFORM->window->windowsize.y = height;
}

void engine_window_resize_pointers(i32* w, i32* h) {
  GLOBAL_PLATFORM->window->game_w = w;
  GLOBAL_PLATFORM->window->game_h = h;
}

void engine_window_resize_pointers_reset(void) {
  GLOBAL_PLATFORM->window->game_w = NULL;
  GLOBAL_PLATFORM->window->game_h = NULL;
}

void engine_draw_uitree(UITree* t) {
  if (drawcall_len + 1 >= drawcall_limit) return;
  drawcalls[drawcall_len++] = (RenderDrawCall){
      .type = RenderDrawCallType_UITree, .data.data = (void*)t};
}

void engine_draw_sprite(Sprite* s, v2_i32* pos, f32 scale) {
  if (drawcall_len + 1 >= drawcall_limit) return;
#ifdef _DEBUG
  if (s == NULL) __asm__("int3;");
#endif
  drawcalls[drawcall_len++] =
      (RenderDrawCall){.type = RenderDrawCallType_Sprite,
                       .data.sprite = {
                           .sprite = s,
                           .x = pos->x,
                           .y = pos->y,
                           .scale = scale,
                           //.mod = {0xFF, 0xFF, 0xFF, 0xFF},
                       }};
}

void engine_draw_sprite_ex(Sprite* s, v2_i32* pos, f32 scale,
                           Engine_color colormod) {
  if (drawcall_len + 1 >= drawcall_limit) return;
#ifdef _DEBUG
  if (s == NULL) __asm__("int3;");
#endif
  drawcalls[drawcall_len++] = (RenderDrawCall){
      .type = RenderDrawCallType_Sprite,
      .data.sprite = {
          .sprite = s,
          .x = pos->x,
          .y = pos->y,
          .scale = scale,
          //.mod = {colormod.r, colormod.g, colormod.b, colormod.a},
      }};
}

Sprite sprite_new(u64 tid, u8 coord) {
  const i32 ts =
      ((struct Resources*)GLOBAL_PLATFORM->data)->textures[tid]->tilesize;
  return (Sprite){.texture_id = tid,
                  (v2_i32){
                      .x = ts * (coord & 0x0F),
                      .y = ts * ((coord & 0xF0) >> 4),
                  }};
}
