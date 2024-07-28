#ifndef ENGINE_RENDERING_RENDERING_H
#define ENGINE_RENDERING_RENDERING_H

#ifdef __cplusplus
extern "C" {
#endif

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
/* TODO: Cleanup these types. */
typedef struct {
  u8 r;
  u8 g;
  u8 b;
  u8 a;
} Engine_color;

typedef struct {
  /* Maybe implement types, such as `atlas` (default), `standalone`, or
   * something idk. */
  u32 id;
  i32 width;
  i32 height;
} Texture;

typedef struct {
  u32 texture_id;
  v2_i32 coord;
} Sprite;

typedef enum {
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

typedef enum {
  ShaderBufferDataType_nil,

  ShaderBufferDataType_f32,
  ShaderBufferDataType_f64,

  ShaderBufferDataType_i32,
  ShaderBufferDataType_i64,
} ShaderBufferDataType;

typedef struct {
  // The backend ID, ie. glGenBuffer(numBufferObjects, &this->buffername)
  u32 buffername;
  // The size of data  = count * size_elem
  isize size;
  // Number of elements
  isize count;
  // components per generic vertex attribute (ie, 3 for RGB, 2 for UV)
  isize components;
  // Type & size of each element
  ShaderBufferDataType datatype;
  isize size_elem;
  // Pointer to the data
  void* data;
} ShaderBuffer;

#define SHADERBUFFER_NEW(T, COUNT, COMPONENTS, DATA) \
  (ShaderBuffer){ \
    .buffername = 0, \
    .size = COUNT * sizeof(T), \
    .count = COUNT, \
    .components = COMPONENTS, \
    .datatype = ShaderBufferDataType_##T, \
    .size_elem = sizeof(T), \
    .data = DATA, \
  }

typedef struct {
  /* Shader proram */
  Shader shader;
  /* Vertex Array Object */
  u32 vao;

  /* MVP (a uniform from the shader).
   * This could also probably be generalized */
  u32 mvp;

  // The texture ID, glBindTextures(target, &this->texture)
  u32 texture;

  // Number of buffers
  usize buffer_len;

  // The vertex buffer is also just a buffer
  ShaderBuffer* buffer;
} RenderObject;

typedef struct {
  // Size of models buffer
  isize msize;
  // number of models in the `models` buffer
  isize mcount;

  RenderObject *models;

  // Size and count of vertices
  RenderObject renderobj;
} RenderBatch;

typedef enum {
  Camera_Perspective,
  Camera_Orthogonal,
} CameraType;

typedef struct {
  /* Position of the camera in world-space. */
  vec3 pos;

  /* The viewing direction of the camera, relative to the camera. */
  vec3 dir;

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

const
usize ShaderBufferDataType_size(ShaderBufferDataType t);

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

void engine_draw_sprite(Sprite* s, v2_i32* pos, f32 scale);
void engine_draw_sprite_ex(Sprite* s, v2_i32* pos, f32 scale,
                           Engine_color colormod);
void engine_draw_model(RenderObject* o, vec3 pos);

Sprite sprite_new(u64 tid, u8 coord);

typedef enum {
  RenderDrawCallType_Text,
  RenderDrawCallType_Sprite,
  RenderDrawCallType_Model,
  RenderDrawCallType_Batch,
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
    struct {
      RenderObject* model;
      isize id;
    } batch;
  } data;
} RenderDrawCall;

// TODO make all the shader buffers a list

RenderObject RenderObject_new(
    Shader* shader,
    u32 texture,
    ShaderBuffer *restrict buffers, usize num_buffers);

Shader compile_shader(const char* file_path, const ShaderType shader_type);
Shader compose_shader(Shader *shaders, usize shaders_len);
void shaders_delete(Shader* shader, isize shader_len);

u32 ComposeShader(u32 *shaders, usize shaders_len);

ShaderType guess_shadertype_from_filename(const char *restrict fname);

Texture createTextureFromImageData(unsigned char* image_data, i32 width, i32 height);

#ifdef __cplusplus
}
#endif
#endif
