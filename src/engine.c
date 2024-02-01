#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//#define GLAD_GL_IMPLEMENTATION
//#include <glad/gl.h>
//#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
/* include winapi */
#include <Windows.h>
#elif defined(__APPLE__)
/* mac includes */
#elif defined(__linux) || defined(__linux__) || defined(linux)

#include <unistd.h>
#include <sys/sysinfo.h>

#endif

#define ENGINE_INTERNALS
#include <engine/btree.h>
#include <engine/engine.h>
#include <engine/hashmap.h>
#include <engine/list.h>

#include <engine/state.h>
// #include <states/titlescreen.h>
// #include <states/gameover.h>

#define DEFAULT_NUM_PROCS 8

#ifdef BENCHMARK
#define BENCHEXPR(timevar, expr)                                               \
  {                                                                            \
    u32 t = glfwGetTime();                                                    \
    expr timevar += glfwGetTime() - t;                                        \
  }

extern i32 drawcall_len;

#else
#define BENCHEXPR(timevar, expr) expr
#endif


// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	char* VertexShaderCode;
	FILE* VertexShaderStream = fopen(vertex_file_path, "r");
	if(VertexShaderStream != NULL){
    fseek(VertexShaderStream, 0, SEEK_END);
    const i64 size = ftell(VertexShaderStream);
    rewind(VertexShaderStream);
    VertexShaderCode = calloc(size + 1, sizeof(char));

    // Assume the whole file is successfully read
    fread(VertexShaderCode, sizeof(char), size, VertexShaderStream);
    //LOG("vertex source is %d bytes\n%s\n", size, VertexShaderCode);

    fclose(VertexShaderStream);
  } else {
    ERROR("Impossible to open %s. Are you in the right directory?\n", vertex_file_path);
    getchar();
    return 0;
  }

	// Read the Fragment Shader code from the file
	char* FragmentShaderCode;
	FILE* FragmentShaderStream = fopen(fragment_file_path, "r");
	if(FragmentShaderStream != NULL){
    fseek(FragmentShaderStream, 0, SEEK_END);
    const i64 size = ftell(FragmentShaderStream);
    rewind(FragmentShaderStream);
    FragmentShaderCode = calloc(size + 1, sizeof(char));

    // Assume the whole file is successfully read
    fread(FragmentShaderCode, sizeof(char), size, FragmentShaderStream);
    LOG("fragment source is %d bytes", size);

    fclose(FragmentShaderStream);
  } else {
    ERROR("Impossible to open %s. Are you in the right directory?\n", fragment_file_path);
    getchar();
    return 0;
  }

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	INFO("Compiling shader: %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode;
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ) {
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, msg);
    ERROR("Compiling shader: %s\n", msg);
    free(msg);
	}

	// Compile Fragment Shader
	INFO("Compiling shader: %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode;
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, msg);
    ERROR("Compiling shader: %s\n", msg);
    free(msg);
	}

	// Link the program
	INFO("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    glGetShaderInfoLog(ProgramID, InfoLogLength, NULL, msg);
    ERROR("Compiling shader: %s\n", msg);
    free(msg);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);


  free(VertexShaderCode);
  free(FragmentShaderCode);
	return ProgramID;
}











static u64 FPS_CAP = 50;
Platform* GLOBAL_PLATFORM = NULL;

input_callback_t* callbacks[128];
usize callbacks_len;

i32 nproc(void) {
  return get_nprocs();
}

void delay( uint32_t ms )
{
#ifdef _WIN32
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

v2_i32 get_canvas_size(void* window) {
  v2_i32 realsize;
  glfwGetWindowSize(window, &(realsize.x), &(realsize.y));

  /* Set logical render size */
  return realsize;
}

Texture* load_texture(void* render, const Asset_TextureSpec* ts) {
  Texture* t = NULL;

  if (ts == NULL) {
    ERROR("Invalid Asset_TextureSpec\n");
    return NULL;
  }

  //t = (Texture*)malloc(sizeof(Texture));
  //t->texture = new_texture;
  ///* Assigning const value */
  //*(i32*)&t->tilesize = tw;
  //*(i32*)&t->width = ts->width;
  //*(i32*)&t->height = ts->height;

  return t;
}

void engine_update_window(Window* w, void* e) {
  switch ((i32)e) {
  default:
    WARN("Unhandled window event 0x%04x", (i32)e);
    break;
  }
  return;
}

struct glfw_ctx {
  GLFWwindow* w;
} glfw_ctx;

/* GLFW And vulkan spaghetti boiler */
void glfw_err_callback(int code, const char* description) {
    ERROR("glfw [%d]: %s\n", code, description);
    // Terminate?
    exit(EXIT_FAILURE);
}

struct QueueFamilyIndices {
  int64_t graphicsFamily;
  int64_t presentFamily;
};

//GladGLContext* create_context(GLFWwindow *window) {
//    glfwMakeContextCurrent(window);
//
//    GladGLContext* context = (GladGLContext*) calloc(1, sizeof(GladGLContext));
//    if (!context) return NULL;
//
//    int version = gladLoadGLContext(context, glfwGetProcAddress);
//    INFO("Loaded OpenGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
//
//    return context;
//}

struct glfw_ctx initialize_GLFW(
    const char* windowtitle, v2_i32 windowsize,
    const u32 flags
    ) {
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

  glfwWindowHint(GLFW_SAMPLES, 4); // Disable anti aliasing

  // Use a modern opengl version
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


  /* "On Wayland specifically, you need to swap the buffers
   *  of a window for it to become visible." */
  window = glfwCreateWindow(windowsize.x, windowsize.y, windowtitle, NULL, NULL);
  if (window == NULL) {
    ERROR("Failed to create GLFW window!\n");
    const char *desc;
    int code = glfwGetError(&desc);
    ERROR("failed to initialize glfw window [%d]: %s\n", code, desc);
    exit(EXIT_FAILURE);
  } else
    printf("ok\n");

  glfwMakeContextCurrent(window);

  // Remember to load GL :) (hours wasted because i forgot: approx 4)
  int version = gladLoadGL(glfwGetProcAddress);
  //printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));


#ifdef _DEBUG
  glClearColor((float)0x10 / 255.f, (float)0x0a / 255.f, (float)0x33 / 255.f, 0.f);
#else
  glClearColor(0x0, 0x0, 0x0, 0.f);
#endif

  // TODO: Replace with callback
  //glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  //while(!(glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS)) {

  //glClear( GL_COLOR_BUFFER_BIT );

  //glfwSwapBuffers(window);

  //glfwPollEvents();
  //}

  return (struct glfw_ctx){.w = window};
}


/* Creates the window, initializes IO, Rendering, Fonts and engine-specific
 * resources. */
Platform* engine_init(const char* windowtitle, v2_i32 windowsize,
                      const f32 render_scale, const u32 flags,
                      const usize initial_memory, const Asset_FontSpec* fonts[],
                      const Asset_TextureSpec* textures[]) {

#ifdef BENCHMARK
  u32 init_start = glfwGetTime();
#endif

#if defined(__linux) || defined(__linux__) || defined(linux)
  {
    pid_t pid = getpid();
    INFO("Starting with pid %lu", pid);
  }
#endif

  Platform* p = (Platform*)malloc(sizeof(Platform));
  Window* w = (Window*)malloc(sizeof(Window));

  /* initialize resources */
  struct Resources* resources =
      (struct Resources*)malloc(sizeof(struct Resources));
  resources->textures_len = 0;
  resources->fonts_len = 0;
  resources->texturepaths_len = 0;
  resources->fontpaths_len = 0;
  resources->texture_paths = NULL;
  resources->font_paths = NULL;
  resources->textures = NULL;
  //resources->fonts = NULL;

  {
    struct glfw_ctx ctx = initialize_GLFW(windowtitle, windowsize, flags);
    w->window = ctx.w;
  }


  struct RenderObject *testobject = malloc(sizeof(struct RenderObject));

  glGenVertexArrays(1, &(testobject->vao));
  glBindVertexArray(testobject->vao);

  p->testobject = testobject;

  testobject->g_vertex_buffer_data[0] = -1.0f;
  testobject->g_vertex_buffer_data[1] = -1.0f;
  testobject->g_vertex_buffer_data[2] =  0.0f;

  testobject->g_vertex_buffer_data[3] =  1.0f;
  testobject->g_vertex_buffer_data[4] = -1.0f;
  testobject->g_vertex_buffer_data[5] =  0.0f;

  testobject->g_vertex_buffer_data[6] =  0.0f;
  testobject->g_vertex_buffer_data[7] = -1.0f;
  testobject->g_vertex_buffer_data[8] =  0.0f;

  testobject->shaderprogram = LoadShaders("shader.vertexshader", "shader.fragmentshader");
  INFO("Shaderprogram %d", testobject->shaderprogram);

  // Generate 1 buffer, put the resulting identifier in vertexbuffer
  glGenBuffers(1, &(testobject->vbo));
  // The following commands will talk about our 'vertexbuffer' buffer
  glBindBuffer(GL_ARRAY_BUFFER, testobject->vbo);
  // Give our vertices to OpenGL.
  glBufferData(GL_ARRAY_BUFFER, sizeof(testobject->g_vertex_buffer_data), testobject->g_vertex_buffer_data, GL_STATIC_DRAW);

  { /* Resource loading */

    /* Count resources */
    usize n_textures = 0;
    usize n_fonts = 0;

    if (textures != NULL)
      while (textures[n_textures] != NULL) n_textures++;
    if (fonts != NULL)
      while (fonts[n_fonts] != NULL) n_fonts++;

    INFO("Number of textures: " TERM_COLOR_YELLOW "%d" TERM_COLOR_RESET,
         n_textures);
    INFO("Number of fonts: " TERM_COLOR_YELLOW "%d" TERM_COLOR_RESET, n_fonts);

    /* Save the textures and fonts, if we should need to reload them later */
    resources->texture_paths = (Asset_TextureSpec**)textures;
    resources->font_paths = (Asset_FontSpec**)fonts;

    /* Allocate memory for textures and fonts */
    resources->textures = (Texture**)malloc(sizeof(Texture*) * n_textures);
    //resources->fonts = (TTF_Font**)malloc(sizeof(TTF_Font*) * n_fonts);
    resources->textures_size = n_textures;

    for (usize i = 0; i < n_textures; i++) resources->textures[i] = NULL;
    //for (usize i = 0; i < n_fonts; i++) resources->fonts[i] = NULL;

    /* Load textures */
    for (usize i = 0; i < n_textures; i++) {
      Texture* t = NULL;
      INFO_("loading texture \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET
            "\"...",
            textures[i]->path);

//      t = load_texture(renderer, textures[i]);
//      if (t == NULL) {
//        puts("");
//        ERROR("failed to load texture\n");
//        exit(EXIT_FAILURE);
//      }
//
//      if (t->tilesize < 8) {
//        puts("");
//        ERROR("texture too small!\n");
//        exit(EXIT_FAILURE);
//      }
//
      //if (t->texture == NULL) {
      //  puts("");
      //  ERROR("failed to load texture\n");
      //} else {
      //  printf("ok\n");
      //  resources->textures[i] = t;
      //  resources->textures_len++;
      //}
    }

    /* Load fonts */
    for (usize i = 0; i < n_fonts; i++) {
      INFO_("loading font \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET "\"...",
            fonts[i]->font_path);

      //TTF_Font* font = TTF_OpenFont(fonts[i]->font_path, fonts[i]->ptsize);
      //if (!font) {
      //  ERROR("failed to load font: %s\n", TTF_GetError());
      //} else {
      //  printf("ok\n");
      //  resources->fonts[i] = font;
      //  resources->fonts_len++;
      //}
    }

    if (resources->textures_len != n_textures) {
      WARN("Done. %d/%d textures loaded.", resources->textures_len, n_textures);
    } else {
      INFO("Done. All %d textures loaded.", n_textures);
    }

    if (resources->fonts_len != n_fonts) {
      WARN("Done. %d/%d fonts loaded.", resources->fonts_len, n_fonts);
    } else {
      INFO("Done. All %d fonts loaded.", n_fonts);
    }

    resources->texturepaths_len = resources->textures_len;
    resources->fontpaths_len = resources->fonts_len;
  }

  { /* Adjust window and such */
    /* Set actual windowsize, which might be forced by OS */
    INFO("Adjusting window size...");
    //windowsize = get_canvas_size(renderer);

    INFO("Windowsize: <%d,%d>", windowsize.x, windowsize.y);
  }

  //w->renderer = renderer;
  //w->window = window;
  w->render_scale = render_scale;
  w->windowsize = windowsize;
  w->game_w = NULL;
  w->game_h = NULL;

  p->data = (void*)resources;
  p->data_len = sizeof(struct Resources);
  p->window = w;
  p->quit = false;

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

#ifdef BENCHMARK
  u32 init_stop = glfwGetTime();
  INFO("Initialization took %dms", init_stop - init_start);
#endif

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

i32 engine_run(Platform* p, StateType initial_state) {
  if (p == NULL) {
    ERROR("Platform is uninitialized.\n");
    INFO("initialize with `engine_init`");
    return -1;
  }

  memory* mem = p->mem;

  StateType state = initial_state;

  {
    u32 state_init_time = glfwGetTime();
    State_init(state, mem);
    INFO("Initializing state \"%s\" took %ldms", StateTypeStr[state],
         glfwGetTime() - state_init_time);
  }

  u32 time = glfwGetTime();

  // Update ticks
  u64 ticks = 0;

  /* Profiling values */
#ifdef BENCHMARK
  u64 profile_tick_counter = 0;
  // u64 profile_slack          = 0;
  u64 profile_rendering = 0;
  u64 profile_gameloop = 0;
  u64 profile_input = 0;
  u64 profile_input_handling = 0;
  u64 profile_num_drawcalls = 0;
  u32 profile_interval_timer = time;
  const u32 profile_interval_ms = 5000;
  const f32 profile_interval_scale = (f32)(profile_interval_ms) / 100.0f;
#endif

  const f64 frame_interval = 1000.0 / FPS_CAP;

  StateType (*update_func)(void*) = State_updateFunc(state);

  /* Main loop */
  INFO("Program: %d", p->testobject->shaderprogram);
  do {
    const u32 now = glfwGetTime();
    const u64 dt = now - time;
    time = now;
    /* Wait frame_interval */
    if (dt < frame_interval) {
#ifndef BENCHMARK
      delay(frame_interval - dt);

#else
      /* We want to know how much time is spend sleeping */
      // profile_slack += frame_interval - dt;
#endif
    }

#ifdef BENCHMARK
    if (time - profile_interval_timer > profile_interval_ms) {
      /* Ticks/frames since last measurement */
      u32 fps = (ticks - profile_tick_counter) / profile_interval_scale;
      u64 drawcalls = profile_num_drawcalls / profile_interval_scale / fps;

      u32 sum = +profile_rendering
                //+ profile_slack
                + profile_input + profile_input_handling + profile_gameloop;

      /* Log fps and slack percentage */
      LOG("fps:%d\t"
          "rendering:%.2f%%\t"
          //"slack:%.2f%%\t"
          "input:%.2f%% (%.2f%%)\t"
          "gameloop:%.2f%%\t"
          "unaccounted:%llu / %llu ms\t"
          "avg drawcalls:%llu",
          fps, 100.0f * (f32)profile_rendering / (f32)sum,
          // 100.0f * (f32)profile_slack          / (f32)sum,
          100.0f * (f32)profile_input / (f32)sum,
          100.0f * (f32)profile_input_handling / (f32)sum,
          100.0f * (f32)profile_gameloop / (f32)sum,
          time - profile_interval_timer - sum, sum, drawcalls);
      /* Reset values */
      profile_tick_counter = ticks;
      profile_interval_timer = time;
      // profile_slack          = 0;
      profile_rendering = 0;
      profile_gameloop = 0;
      profile_input = 0;
      profile_input_handling = 0;
      profile_num_drawcalls = 0;
    }
#endif

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
//    StateType next_state;
//      next_state = update_func((void*)(mem->data));
//
//    if (next_state != STATE_null) {
//      if (next_state == STATE_quit) break;
//
//      drawcall_reset();
//
//      engine_window_resize_pointers_reset();
//      State_free(state, mem);
//      memory_clear(mem);
//
//      engine_input_ctx_reset();
//
//      state = next_state;
//      update_func = State_updateFunc(state);
//#ifdef BENCHMARK
//      {
//        f64 t = glfwGetTime();
//        State_init(state, mem);
//        LOG("Initializing %s took %dms", StateTypeStr[state],
//            (int)((glfwGetTime() - t) * 1000.0));
//      }
//#else
//      State_init(state, mem);
//#endif
//    } else {
//#ifdef BENCHMARK
//      profile_num_drawcalls += drawcall_len;
//#endif
//      //render_begin(p->window);
  glClear(GL_COLOR_BUFFER_BIT );

      glUseProgram(p->testobject->shaderprogram);
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, p->testobject->vbo);
      glVertexAttribPointer(
          0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
          3,                  // size
          GL_FLOAT,           // type
          GL_FALSE,           // normalized?
          0,                  // stride
          (void*)0            // array buffer offset
          );
      glUseProgram(p->testobject->shaderprogram);
      // Draw the triangle !
      glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
      glDisableVertexAttribArray(0);
  glfwSwapBuffers(p->window->window);
      //render_present(p->window);
//    }

    ticks++;
  } while(
      !glfwWindowShouldClose(p->window->window)
      && state != STATE_quit);

  return 0;
}

void stop(Platform* p) {
  if (p == NULL) return;

  { /* Deallocate resources */
    struct Resources* r = (struct Resources*)p->data;
    if (r != NULL) {
      /* Destroy textures */
      for (usize i = 0; i < r->textures_len; i++) {
        if (r->textures[i] != NULL) {
          r->textures[i] = NULL;
        }
      }
      free(r->textures);

      /* Destroy Fonts */
    }
  }

  glfwDestroyWindow(p->window->window);
  glfwTerminate();

}

/* Set the maximum framerate */
void engine_fps_max(u64 cap) { FPS_CAP = cap; }

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

u32 get_time(void) { return glfwGetTime(); }
v2_i32 get_windowsize(void) { return GLOBAL_PLATFORM->window->windowsize; }
v2_i32* get_mousepos(void) { return &GLOBAL_PLATFORM->mouse_pos; }
