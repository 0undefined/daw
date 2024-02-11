#ifndef WINDOW_H
#define WINDOW_H

#include <engine/core/types.h>
#include <cglm/cglm.h>

#ifndef ENGINE_RENDERING_WINDOW_H_EXCLUDE_EXTERNS
extern void* window_poll_events;
extern f64 (*get_time)(void);
#endif

typedef enum {
  WINDOW_FRAMEWORK_NONE = 0,
  WINDOW_FRAMEWORK_GLFW,
} Window_framework;

typedef enum {
  WINDOW_RENDERER_NONE = 0,
  WINDOW_RENDERER_OPENGL,
} Window_renderer;

typedef struct {
  // Specifies the framwork & renderer combo used.
  Window_framework framework;
  Window_renderer renderer;
  // Window *buffer* size, in pixels.
  ivec2 windowsize;

  // These are used differently depending on the framework / renderer combo.
  // Subject to change to a union of backend-dependent structs
  void* window;
  void* context;
} Window;

// Window function
Window* init_window_glfw(const char* windowtitle, ivec2 windowsize, const u32 flags);
void destroy_window(Window* w);

// Renderer intializer(s)
void init_render_opengl(Window* w);

#undef API
#endif
