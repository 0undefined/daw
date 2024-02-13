#ifndef ENGINE_RENDERING_RENDERING_H
#define ENGINE_RENDERING_RENDERING_H

#include <engine/core/types.h>
#include <engine/utils/vector.h>
#include <engine/rendering/window.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>

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

typedef enum {
  //GL_COMPUTE_SHADER, GL_VERTEX_SHADER, GL_TESS_CONTROL_SHADER, GL_TESS_EVALUATION_SHADER, GL_GEOMETRY_SHADER, GL_FRAGMENT_SHADER
  Shader_Error,
  Shader_Program, /* Collection of shaders */
  Shader_Vertex,
  Shader_Tessellation,
  Shader_Geometry,
  Shader_Fragment,
  Shader_Compute,
} ShaderType;

typedef struct {
  /* Shader proram */
  ShaderType type;
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

typedef enum {
  Camera_Perspective,
  Camera_Orthogonal,
} CameraType;

typedef struct {
  /* Position of the camera in world-space. */
  vec3 pos;
  /* Perspective matrix. Initialize with r_perspective_ortho or r_perspective. */
  /* Alternatively, use `glm_perspective` or `glm_ortho`. */
  mat4 per;

  /* Used to re-calculate the perspective matrix when resizing the window */
  CameraType type;
  /* Yes, could use a singular "f32 arg", but this is more extendable in the
   * future. */
  union {
    struct {f32 fov;} perspective;
    struct {f32 sz;} orthogonal;
  } parameters;

} Camera;

/* Rendering functions */
void render_begin(Window* w);
void render_present(Window* w);
void drawcall_reset(void);
void render(Window* w);

/* Misc */
void r_perspective(f32 fov, Camera *c);
void r_perspective_ortho(f32 sz, Camera *c);

void r_set_camera(Camera* c);

//void window_size_callback(GLFWwindow* window, i32 width, i32 height);
//void engine_window_resize_pointers(i32* w, i32* h);
//void engine_window_resize_pointers_reset(void);

void engine_draw_sprite(Sprite* s, v2_i32* pos, f32 scale);
void engine_draw_sprite_ex(Sprite* s, v2_i32* pos, f32 scale,
                           Engine_color colormod);
void engine_draw_model(RenderObject* o, vec3 pos);

Sprite sprite_new(u64 tid, u8 coord);

typedef enum {
  RenderDrawCallType_Text,
  RenderDrawCallType_Sprite,
  RenderDrawCallType_Model,
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
    struct {
      RenderObject* model;
      vec3 pos;
      f32 scale;
    } model;
  } data;
} RenderDrawCall;

RenderObject RenderObject_new(float* model, Shader* shader, usize sz, float* uv, usize uv_sz);

Shader compile_shader(const char* file_path, const ShaderType shader_type);
Shader compose_shader(Shader *shaders, usize shaders_len);

u32 ComposeShader(u32 *shaders, usize shaders_len);

ShaderType guess_shadertype_from_filename(const char *restrict fname);

#endif
