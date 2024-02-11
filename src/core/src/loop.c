#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <cglm/cglm.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* include winapi */
#include <Windows.h>
#elif defined(__APPLE__)
/* mac includes */
#elif defined(__linux) || defined(__linux__) || defined(linux)

#include <unistd.h>
#include <sys/sysinfo.h>

#endif

#include <engine/core/state.h>
#include <engine/engine.h>
#include <engine/utils/btree.h>
#include <engine/utils/hashmap.h>
#include <engine/utils/list.h>

#include <engine/resources.h>

#include <engine/rendering/window.h>
#include <engine/rendering/rendering.h>


#define DEFAULT_NUM_PROCS 8

Platform* GLOBAL_PLATFORM = NULL;

input_callback_t* callbacks[128];
usize callbacks_len;

i32 nproc(void) {
  return get_nprocs();
}

void delay( uint32_t ms )
{
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    Sleep( ms );
#else
    usleep( ms * 1000 );
#endif
}

i32 cmp_int(const void* a, const void* b) {
  const i32* x = a;
  const i32* y = b;

  return *x - *y;
}

//v2_i32 get_canvas_size(void* window) {
//  v2_i32 realsize;
//  glfwGetWindowSize(window, &(realsize.x), &(realsize.y));
//
//  /* Set logical render size */
//  return realsize;
//}

void engine_update_window(Window* w, void* e) {
  //WARN("Unhandled function", (i32)e);
  return;
}


/* Creates the window, initializes IO, Rendering, Fonts and engine-specific
 * resources. */
Platform* engine_init(const char* windowtitle, i32 windowWidth, i32 windowHeight,
                      const f32 render_scale, const u32 flags,
                      const usize initial_memory) {

#if defined(__linux) || defined(__linux__) || defined(linux)
  {
    pid_t pid = getpid();
    INFO("Starting with pid %lu", pid);
  }
#endif
 v2_i32 windowsize = (v2_i32){.x = windowWidth, .y = windowHeight};

  Platform* p = (Platform*)malloc(sizeof(Platform));
  Window w = (Window)malloc(sizeof(Window));

  /* initialize resources */
  Resources* resources = malloc(sizeof(Resources));
  // TODO: Initialize them :)

  w = init_window_glfw(windowtitle, (ivec2){windowsize.x, windowsize.y}, flags);
  // Dont forget to init the renderer
  init_render_opengl(w);

  //const GladGLContext *gl = w->context;

  //struct RenderObject *testobject = malloc(sizeof(struct RenderObject));

  //gl->GenVertexArrays(1, &(testobject->vao));
  //gl->BindVertexArray(testobject->vao);

  //p->testobject = testobject;

  //testobject->g_vertex_buffer_data[0] = -1.0f;
  //testobject->g_vertex_buffer_data[1] = -1.0f;
  //testobject->g_vertex_buffer_data[2] =  0.0f;

  //testobject->g_vertex_buffer_data[3] =  1.0f;
  //testobject->g_vertex_buffer_data[4] = -1.0f;
  //testobject->g_vertex_buffer_data[5] =  0.0f;

  //testobject->g_vertex_buffer_data[6] =  0.0f;
  //testobject->g_vertex_buffer_data[7] =  1.0f;
  //testobject->g_vertex_buffer_data[8] =  0.0f;

  //static const float bufdata[] = {
  //  -1.0f,-1.0f,-1.0f, // triangle 1 : begin
  //  -1.0f,-1.0f, 1.0f,
  //  -1.0f, 1.0f, 1.0f, // triangle 1 : end
  //  1.0f, 1.0f,-1.0f, // triangle 2 : begin
  //  -1.0f,-1.0f,-1.0f,
  //  -1.0f, 1.0f,-1.0f, // triangle 2 : end
  //  1.0f,-1.0f, 1.0f,
  //  -1.0f,-1.0f,-1.0f,
  //  1.0f,-1.0f,-1.0f,
  //  1.0f, 1.0f,-1.0f,
  //  1.0f,-1.0f,-1.0f,
  //  -1.0f,-1.0f,-1.0f,
  //  -1.0f,-1.0f,-1.0f,
  //  -1.0f, 1.0f, 1.0f,
  //  -1.0f, 1.0f,-1.0f,
  //  1.0f,-1.0f, 1.0f,
  //  -1.0f,-1.0f, 1.0f,
  //  -1.0f,-1.0f,-1.0f,
  //  -1.0f, 1.0f, 1.0f,
  //  -1.0f,-1.0f, 1.0f,
  //  1.0f,-1.0f, 1.0f,
  //  1.0f, 1.0f, 1.0f,
  //  1.0f,-1.0f,-1.0f,
  //  1.0f, 1.0f,-1.0f,
  //  1.0f,-1.0f,-1.0f,
  //  1.0f, 1.0f, 1.0f,
  //  1.0f,-1.0f, 1.0f,
  //  1.0f, 1.0f, 1.0f,
  //  1.0f, 1.0f,-1.0f,
  //  -1.0f, 1.0f,-1.0f,
  //  1.0f, 1.0f, 1.0f,
  //  -1.0f, 1.0f,-1.0f,
  //  -1.0f, 1.0f, 1.0f,
  //  1.0f, 1.0f, 1.0f,
  //  -1.0f, 1.0f, 1.0f,
  //  1.0f,-1.0f, 1.0f
  //};

  //static const GLfloat g_color_buffer_data[] = {
  //  0.583f,  0.771f,  0.014f,
  //  0.609f,  0.115f,  0.436f,
  //  0.327f,  0.483f,  0.844f,
  //  0.822f,  0.569f,  0.201f,
  //  0.435f,  0.602f,  0.223f,
  //  0.310f,  0.747f,  0.185f,
  //  0.597f,  0.770f,  0.761f,
  //  0.559f,  0.436f,  0.730f,
  //  0.359f,  0.583f,  0.152f,
  //  0.483f,  0.596f,  0.789f,
  //  0.559f,  0.861f,  0.639f,
  //  0.195f,  0.548f,  0.859f,
  //  0.014f,  0.184f,  0.576f,
  //  0.771f,  0.328f,  0.970f,
  //  0.406f,  0.615f,  0.116f,
  //  0.676f,  0.977f,  0.133f,
  //  0.971f,  0.572f,  0.833f,
  //  0.140f,  0.616f,  0.489f,
  //  0.997f,  0.513f,  0.064f,
  //  0.945f,  0.719f,  0.592f,
  //  0.543f,  0.021f,  0.978f,
  //  0.279f,  0.317f,  0.505f,
  //  0.167f,  0.620f,  0.077f,
  //  0.347f,  0.857f,  0.137f,
  //  0.055f,  0.953f,  0.042f,
  //  0.714f,  0.505f,  0.345f,
  //  0.783f,  0.290f,  0.734f,
  //  0.722f,  0.645f,  0.174f,
  //  0.302f,  0.455f,  0.848f,
  //  0.225f,  0.587f,  0.040f,
  //  0.517f,  0.713f,  0.338f,
  //  0.053f,  0.959f,  0.120f,
  //  0.393f,  0.621f,  0.362f,
  //  0.673f,  0.211f,  0.457f,
  //  0.820f,  0.883f,  0.371f,
  //  0.982f,  0.099f,  0.879f
  //};


////  LOG("sizeof(bufdata) = %lu", sizeof(bufdata));
////  LOG("sizeof(g_vertex_buffer_data) = %lu", sizeof(testobject->g_vertex_buffer_data));
////

  //// Generate 1 buffer, put the resulting identifier in vertexbuffer
  //gl->GenBuffers(1, &(testobject->vbo));
  //// The following commands will talk about our 'vertexbuffer' buffer
  //gl->BindBuffer(GL_ARRAY_BUFFER, testobject->vbo);
  //// Give our vertices to OpenGL.
  //gl->BufferData(GL_ARRAY_BUFFER, sizeof(bufdata), bufdata, GL_STATIC_DRAW);

  //// Same for the color buffer
  //gl->GenBuffers(1, &(testobject->col));
  //gl->BindBuffer(GL_ARRAY_BUFFER, testobject->col);
  //gl->BufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

  //testobject->shaderprogram = LoadShaders(gl, "shader.vertexshader", "shader.fragmentshader");
  //INFO("Shaderprogram %d", testobject->shaderprogram);

  //{ /* Resource loading */

  //  /* Count resources */
  //  usize n_textures = 0;
  //  usize n_fonts = 0;

  //  if (textures != NULL)
  //    while (textures[n_textures] != NULL) n_textures++;
  //  if (fonts != NULL)
  //    while (fonts[n_fonts] != NULL) n_fonts++;

  //  INFO("Number of textures: " TERM_COLOR_YELLOW "%d" TERM_COLOR_RESET,
  //       n_textures);
  //  INFO("Number of fonts: " TERM_COLOR_YELLOW "%d" TERM_COLOR_RESET, n_fonts);

  //  /* Save the textures and fonts, if we should need to reload them later */
  //  resources->texture_paths = (Asset_TextureSpec**)textures;
  //  resources->font_paths = (Asset_FontSpec**)fonts;

  //  /* Allocate memory for textures and fonts */
  //  resources->textures = (Texture**)malloc(sizeof(Texture*) * n_textures);
  //  //resources->fonts = (TTF_Font**)malloc(sizeof(TTF_Font*) * n_fonts);
  //  resources->textures_size = n_textures;

  //  for (usize i = 0; i < n_textures; i++) resources->textures[i] = NULL;
  //  //for (usize i = 0; i < n_fonts; i++) resources->fonts[i] = NULL;

  //  /* Load textures */
  //  for (usize i = 0; i < n_textures; i++) {
  //    Texture* t = NULL;
  //    INFO_("loading texture \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET
  //          "\"...",
  //          textures[i]->path);

////      t = load_texture(renderer, textures[i]);
////      if (t == NULL) {
////        puts("");
////        ERROR("failed to load texture\n");
////        exit(EXIT_FAILURE);
////      }
////
////      if (t->tilesize < 8) {
////        puts("");
////        ERROR("texture too small!\n");
////        exit(EXIT_FAILURE);
////      }
////
  //    //if (t->texture == NULL) {
  //    //  puts("");
  //    //  ERROR("failed to load texture\n");
  //    //} else {
  //    //  printf("ok\n");
  //    //  resources->textures[i] = t;
  //    //  resources->textures_len++;
  //    //}
  //  }

  //  /* Load fonts */
  //  for (usize i = 0; i < n_fonts; i++) {
  //    INFO_("loading font \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET "\"...",
  //          fonts[i]->font_path);

  //    //TTF_Font* font = TTF_OpenFont(fonts[i]->font_path, fonts[i]->ptsize);
  //    //if (!font) {
  //    //  ERROR("failed to load font: %s\n", TTF_GetError());
  //    //} else {
  //    //  printf("ok\n");
  //    //  resources->fonts[i] = font;
  //    //  resources->fonts_len++;
  //    //}
  //  }

  //  if (resources->textures_len != n_textures) {
  //    WARN("Done. %d/%d textures loaded.", resources->textures_len, n_textures);
  //  } else {
  //    INFO("Done. All %d textures loaded.", n_textures);
  //  }

  //  if (resources->fonts_len != n_fonts) {
  //    WARN("Done. %d/%d fonts loaded.", resources->fonts_len, n_fonts);
  //  } else {
  //    INFO("Done. All %d fonts loaded.", n_fonts);
  //  }

  //  resources->texturepaths_len = resources->textures_len;
  //  resources->fontpaths_len = resources->fonts_len;
  //}

  //{ /* Adjust window and such */
  //  /* Set actual windowsize, which might be forced by OS */
  //  INFO("Adjusting window size...");
  //  //windowsize = get_canvas_size(renderer);

  //  INFO("Windowsize: <%d,%d>", windowsize.x, windowsize.y);
  //}

  p->window = w;
  p->quit = false;

  p->resources = resources;

  p->frame = 0;
  p->fps_target = 60;

  p->mem = memory_new(initial_memory);

  /* Getting the mouse coords now resolves the issue where a click "isn't
   * registered" when the mouse isn't moved before the user clicks */

  p->mousedown = (v2_i32){-1, -1};
  p->mouseup = (v2_i32){-1, -1};

  p->mouse_lclick = false;
  p->mouse_rclick = false;

  p->camera_x = 0;
  p->camera_y = 0;

  p->edit_text = NULL;
  p->edit_pos = 0;

  p->bindings = NULL;
  p->bindings_sz = 0;
  p->bindings_len = 0;

  // TODO: Add global bindings

  INFO("Available cores: %d", nproc());

  GLOBAL_PLATFORM = p;

#ifdef DAW_BUILD_HOTRELOAD

#define State(name)                                                            \
  if (!State_reload(STATE_##name, p->bindings, p->bindings_len)) {             \
    ERROR("Failed to reload shared object file for state %s", #name);          \
  };

#include <states/list_of_states.h>
#undef State

#endif

  return p;
}

i32 engine_run(Platform* p, StateType initial_state, void* state_arg) {
  if (p == NULL) {
    ERROR("Platform is uninitialized.\n");
    INFO("initialize with `engine_init`");
    return -1;
  }

  memory* mem = p->mem;

  StateType state = initial_state;

  {
    f64 state_init_time = get_time();
    State_init(state, mem, state_arg);
    INFO("Initializing state \"%s\" took %.1fms", StateTypeStr[state],
         (get_time() - state_init_time) * 1000.0);
  }

  f64 time = get_time();

  // Update ticks
  u64 ticks = 0;

  StateType (*update_func)(void*) = State_updateFunc(state);

  f64 last_fps_measurement = get_time();

  /* Main loop */
  do {
    const f64 now = get_time();
    const f64 dt = now - time;
    time = now;

    if (now - last_fps_measurement > 1.000) {
      last_fps_measurement = now;
      //printf("\n FPS: %.1f  \t ticks: %lu", (double)ticks / now, ticks);
    }

    glfwPollEvents();
    /* Events */
//    if (p->mouse_lclick) {
//      p->mouseup.x = -1;
//      p->mouseup.y = -1;
//      p->mousedown.x = -1;
//      p->mousedown.y = -1;
//      p->mouse_lclick = false;
//    }
//    if (p->mouse_rclick) {
//      p->mouse_rclick = false;
//    }
//
//    /* Window events */
//    i32 num_events;
//
//    /* Mouse events */
//
//    if (p->bindings != NULL) {
//      const i_ctx* bindings = *p->bindings;
//      const usize bindings_len = p->bindings_len;
//
//    }
//
//    i_flush_bindings(callbacks_len, mem->data, callbacks);
//    callbacks_len = 0;
//
//    /* update */
    StateType next_state;
      next_state = update_func((void*)(mem->data));

    if (next_state != STATE_null) {
      if (next_state == STATE_quit) break;

      drawcall_reset();

      void* retval = State_free(state, mem);
      memory_clear(mem);

      engine_input_ctx_reset();

      state = next_state;
      update_func = State_updateFunc(state);
      {
        f64 state_init_time = get_time();
        State_init(state, mem, retval);
        INFO("Initializing state \"%s\" took %.1fms", StateTypeStr[state],
            (get_time() - state_init_time) * 1000.0);
      }
    } else {

      render_begin(p->window);


      //gl->UseProgram(p->testobject->shaderprogram);

      //{
      //  vec3 cam = {4., 3., 3.}; // perspective
      //  mat4 per; // perspective
      //  mat4 v; // view
      //  mat4 model = GLM_MAT4_IDENTITY_INIT;
      //  mat4 modelviewprojection;

      //  f32 ratio = (float)p->window->windowsize.x / (float)p->window->windowsize.y;
      //  //glm_perspective(45.f , 600.f / 400.f, 0.1, 100.0f, per);
      //  glm_ortho(-10 * ratio, 10 * ratio, -10, 10, -10, 10, per);

      //  glm_lookat(cam, GLM_VEC3_ZERO, GLM_YUP, v);

      //  { mat4 t;
      //    //modelviewprojection = p * v * model
      //    glm_mat4_mul(v, model, t);
      //    glm_rotate_at(t, (vec3){0,0,0}, get_time() / 2.f, (vec3){0,1,0}); //, (vec3)({0,1,0}));
      //    glm_mat4_mul(per, t, modelviewprojection);
      //  }

      //  // TODO: Do this only once during initialization
      //  u32 matrix = gl->GetUniformLocation(p->testobject->shaderprogram, "MVP");

      //  gl->UniformMatrix4fv(matrix, 1, GL_FALSE, &modelviewprojection[0][0]);
      //}


      //gl->EnableVertexAttribArray(0);
      //gl->BindBuffer(GL_ARRAY_BUFFER, p->testobject->vbo);
      //gl->VertexAttribPointer(
      //    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
      //    3,                  // size
      //    GL_FLOAT,           // type
      //    GL_FALSE,           // normalized?
      //    0,                  // stride
      //    (void*)0            // array buffer offset
      //    );

      //// Do the color buffer (?)
      //gl->EnableVertexAttribArray(1);
      //gl->BindBuffer(GL_ARRAY_BUFFER, p->testobject->col);
      //gl->VertexAttribPointer(
      //    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      //    3,                                // size
      //    GL_FLOAT,                         // type
      //    GL_FALSE,                         // normalized?
      //    0,                                // stride
      //    (void*)0                          // array buffer offset
      //    );




      //gl->UseProgram(p->testobject->shaderprogram);
      //// Draw the triangle !
      //gl->DrawArrays(GL_TRIANGLES, 0, 3*12); // Starting from vertex 0; 3 vertices total -> 1 triangle

      //gl->DisableVertexAttribArray(0);
      //gl->DisableVertexAttribArray(1);

      render_present(p->window);
    }

    ticks++;
  } while(
      !glfwWindowShouldClose(p->window->window)
      && state != STATE_quit);

  return 0;
}

void engine_stop(Platform* p) {
  if (p == NULL) return;

  //{ /* Deallocate resources */
  //  struct Resources* r = (struct Resources*)p->data;
  //  if (r != NULL) {
  //    /* Destroy textures */
  //    for (usize i = 0; i < r->textures_len; i++) {
  //      if (r->textures[i] != NULL) {
  //        r->textures[i] = NULL;
  //      }
  //    }
  //    free(r->textures);

  //    /* Destroy Fonts */
  //  }
  //}


  destroy_window(p->window);
}

/* Set the maximum framerate */
void engine_fps_max(u64 cap) { /* does nothing */ }

/* Pushes an input context onto the input handling stack */
void engine_input_ctx_push(i_ctx* ctx) {
  if (GLOBAL_PLATFORM->bindings == NULL) {
    GLOBAL_PLATFORM->bindings = calloc(8, sizeof(i_ctx*));
    GLOBAL_PLATFORM->bindings_sz = 8;
  }

  if (GLOBAL_PLATFORM->bindings_len + 1 >= GLOBAL_PLATFORM->bindings_sz) {
    void* m =
        realloc(GLOBAL_PLATFORM->bindings, GLOBAL_PLATFORM->bindings_sz + 8);
    if (m == NULL) {
      ERROR("Failed to allocate 8 bytes (%d): %s", errno, strerror(errno));
      exit(EXIT_FAILURE);
    }
    GLOBAL_PLATFORM->bindings_sz += 8;
  }

  LOG("Bindings in ctx[%d]:", GLOBAL_PLATFORM->bindings_len);
  for (isize i = 0; i < ctx->len; i++) {
    switch (ctx->bindings[i].action.type) {
    case InputType_error:
      LOG("(error)");
      break;

    case InputType_action:
      LOG("(action) %s", ctx->bindings[i].action.action.callback_str);
      break;

    case InputType_state:
      LOG("(+state) %s", ctx->bindings[i].action.state.activate_str);
      LOG("(-state) %s", ctx->bindings[i].action.state.deactivate_str);
      break;
    case InputType_range:
      LOG("(range) --unhandled--");
      break;
    }
  }

  GLOBAL_PLATFORM->bindings[GLOBAL_PLATFORM->bindings_len++] = ctx;
}

/* Pops an input context from the input stack */
void engine_input_ctx_pop(void) {
  if (GLOBAL_PLATFORM->bindings == NULL || GLOBAL_PLATFORM->bindings_sz == 0)
    return;
  i_ctx_t_free(GLOBAL_PLATFORM->bindings[--GLOBAL_PLATFORM->bindings_len]);
}

/* Removes all input contexts from the input stack */
void engine_input_ctx_reset(void) {
  while (GLOBAL_PLATFORM->bindings_len > 0) {
    i_ctx_t_free(GLOBAL_PLATFORM->bindings[--GLOBAL_PLATFORM->bindings_len]);
  }
}
//ivec2 get_windowsize(void) { return GLOBAL_PLATFORM->window->windowsize; }
v2_i32* get_mousepos(void) { return &GLOBAL_PLATFORM->mouse_pos; }
