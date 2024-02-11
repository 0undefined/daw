#include <stdio.h>

/* TODO: REMOVE THIS INCLUSION */
#include <engine/engine.h>

#include <engine/core/types.h>
#include <engine/core/logging.h>

#define ENGINE_RENDERING_WINDOW_H_EXCLUDE_EXTERNS
#include <engine/rendering/window.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION

#undef GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

extern Platform* GLOBAL_PLATFORM;

void *window_poll_events = NULL;
f64 (*get_time)(void) = NULL;

/* GLFW And vulkan spaghetti boiler */
void glfw_err_callback(int code, const char* description) {
    ERROR("glfw [%d]: %s\n", code, description);
    // Terminate?
    exit(EXIT_FAILURE);
}

void window_size_callback(GLFWwindow* window, int width, int height) {
  GladGLContext* gl = GLOBAL_PLATFORM->window->context;
  gl->Viewport(0,0, width, height);
  GLOBAL_PLATFORM->window->windowsize[0] = width;
  GLOBAL_PLATFORM->window->windowsize[1] = height;
}

GladGLContext* create_context(GLFWwindow *window) {
    glfwMakeContextCurrent(window);

    GladGLContext* context = (GladGLContext*)malloc(sizeof(GladGLContext));
    if (!context) return NULL;

    int version = gladLoadGLContext(context, glfwGetProcAddress);
    INFO("Loaded OpenGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    return context;
}

Window* init_window_glfw(
    const char* windowtitle, ivec2 windowsize,
    const u32 flags
    ) {
  Window* ret = NULL;
  GLFWwindow* window = NULL;

  glfwSetErrorCallback(&glfw_err_callback);

  INFO_("initializing glfw...");
  if (glfwInit() == GLFW_FALSE) {
    const char *desc;
    int code = glfwGetError(&desc);
    ERROR("failed to initialize glfw [%d]: %s\n", code, *desc);
    exit(EXIT_FAILURE);
  } else
    printf("ok\n");


  INFO_("initializing window...");
  //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  glfwWindowHint(GLFW_SAMPLES, 0); // Disable anti aliasing

  // Use a modern opengl version
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  // Lean and mean
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  /* "On Wayland specifically, you need to swap the buffers
   *  of a window for it to become visible." */
  window = glfwCreateWindow(windowsize[0], windowsize[1], windowtitle, NULL, NULL);
  if (window == NULL) {
    ERROR("Failed to create GLFW window!\n");
    const char *desc;
    int code = glfwGetError(&desc);
    ERROR("failed to initialize glfw window [%d]: %s\n", code, desc);
    exit(EXIT_FAILURE);
  } else
    printf("ok\n");

  // Setup callbacks
  // TODO: input handler callback
  glfwSetFramebufferSizeCallback(window, window_size_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  // Create the window datastructure
  ret = (Window*)calloc(1, sizeof(Window));
  ret->framework = WINDOW_FRAMEWORK_GLFW;
  ret->renderer = WINDOW_RENDERER_NONE;
  glm_ivec2_copy(windowsize, ret->windowsize);
  ret->window = window;
  // Last parameter is used for the renderer
  ret->context = NULL;

  window_poll_events = &glfwPollEvents;
  get_time = &glfwGetTime;

  return ret;
}

// Initializes opengl using the window
void init_render_opengl(Window* w) {
  if (w == NULL || w->window == NULL) {
    ERROR("Window is not initialized");
    return;
  }

  if (w->renderer != WINDOW_RENDERER_NONE || w->context != NULL) {
    ERROR("Window already initialized with a renderer!");
    return;
  }

  if (w->framework != WINDOW_FRAMEWORK_GLFW) {
    ERROR("Trying to initialize OpenGL with incompatible window");
    return;
  }

  // This is GLFW specific
  GladGLContext *ctx = create_context((GLFWwindow*)w->window);

  ctx->Viewport(0, 0, w->windowsize[0], w->windowsize[1]);

  if (ctx == NULL) {
    ERROR("Failed to create glad context");
    exit(EXIT_FAILURE);
  }

#ifdef _DEBUG
  ctx->ClearColor((float)0x10 / 255.f, (float)0x0a / 255.f, (float)0x33 / 255.f, 0.f);
#else
  ctx->ClearColor(0x0, 0x0, 0x0, 0.f);
#endif

  // Make sure faces closest to the camera are drawn on-top of faces that are
  // further away
  ctx->Enable(GL_DEPTH_TEST);
  ctx->DepthFunc(GL_LESS);

  w->context = ctx;
  w->renderer = WINDOW_RENDERER_OPENGL;
}

void destroy_window_glfw(GLFWwindow* w) {
  glfwDestroyWindow(w);

  // If we ever do multi-window support, we need to make sure this is the last
  // window before terminating
  glfwTerminate();
}

void destroy_window(Window* w) {
  switch(w->framework) {
    case WINDOW_FRAMEWORK_GLFW:
      destroy_window_glfw(w->window);
      w->window = NULL;
      break;
    default:
      ERROR("Destroying unknown window type.");
  }

  switch(w->renderer) {
    case WINDOW_RENDERER_OPENGL:
      // Missing unloader function in glad MX library
      w->context = NULL;
      break;
    default:
      ERROR("Destroying unknown renderer type.");
  }
}
