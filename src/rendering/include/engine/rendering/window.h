#ifndef WINDOW_H
#define WINDOW_H

#include <engine/core/types.h>
#include <cglm/cglm.h>

enum Window_framework {
  WINDOW_FRAMEWORK_NONE = 0,
  WINDOW_FRAMEWORK_GLFW,
};

typedef enum Window_framework Window_framework;

enum Window_renderer {
  WINDOW_RENDERER_NONE = 0,
  WINDOW_RENDERER_OPENGL,
};

typedef enum Window_renderer Window_renderer;

struct Window {
  // Specifies the framwork & renderer combo used.
  Window_framework framework;
  Window_renderer renderer;
  // Window *buffer* size, in pixels.
  ivec2 windowsize;

  // These are used differently depending on the framework / renderer combo.
  // Subject to change to a union of backend-dependent structs
  void* window;
  void* context;
};

typedef struct Window* Window;

// Window function
Window init_window_glfw(const char* windowtitle, ivec2 windowsize, const u32 flags);
void destroy_window(Window w);

// Renderer intializer(s)
void init_render_opengl(Window w);

#endif
