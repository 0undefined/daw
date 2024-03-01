#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glad/gl.h>

#include <engine/core/types.h>
#include <engine/core/logging.h>

#include <engine/rendering/rendering.h>
#include <engine/core/platform.h>

extern Platform* GLOBAL_PLATFORM;

const char* ShaderType_str[] = {
  [Shader_Error] = "Shader_Error",
  [Shader_Program] = "Shader_Program",
  [Shader_Vertex] = "Shader_Vertex",
  [Shader_Tessellation] = "Shader_Tessellation",
  [Shader_Geometry] = "Shader_Geometry",
  [Shader_Fragment] = "Shader_Fragment",
  [Shader_Compute] = "Shader_Compute",
};

Shader compile_shader(const char* file_path, const ShaderType shader_type) {
  GLuint shaderID = 0;
  GLenum shadertype = GL_INVALID_ENUM;

  GLint Result = GL_FALSE;
  int InfoLogLength;

  char* source = NULL;
  FILE* file = NULL;

  const GladGLContext* gl = GLOBAL_PLATFORM->window->context;

  if (file_path == NULL) {
    WARN("Empty path to shader");
    return (Shader){.program = 0, .type = Shader_Error};
  }

  switch (shader_type) {
    case Shader_Vertex:
      shadertype = GL_VERTEX_SHADER;
      break;
    case Shader_Fragment:
      shadertype = GL_FRAGMENT_SHADER;
      break;
    default: break;
  }

  file = fopen(file_path, "r");

  shaderID = gl->CreateShader(shadertype);
  LOG("CREATED SHADER ID %d", shaderID);

  if(file != NULL) {
    const i64 size = f_get_sz(file);

    source = calloc(size + 1, sizeof(char));

    // Assume the whole file is successfully read
    fread(source, sizeof(char), size, file);

    fclose(file);
  } else {
    ERROR("Cannot open \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET"\".", file_path);
    return (Shader){.program = 0, .type = Shader_Error};
  }

  // Compile shader
  INFO("Compiling shader \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET"\".", file_path);
  char const* src_ptr = source;
  gl->ShaderSource(shaderID, 1, &src_ptr , NULL);
  gl->CompileShader(shaderID);

  // Check shader
  gl->GetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
  gl->GetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ) {
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    gl->GetShaderInfoLog(shaderID, InfoLogLength, NULL, msg);
    ERROR("Failed to compile shader: " TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET, msg);
    free(msg);
  }
  //free(source);

  return (Shader){.program = shaderID, .type = shader_type};
}


// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
GLuint load_shaders(
    const GladGLContext* gl,
    const char* vertex_file_path,
    const char* fragment_file_path) {

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Create the shaders
  const Shader vertexShader = compile_shader(vertex_file_path, Shader_Vertex);
  const Shader fragmentShader = compile_shader(fragment_file_path, Shader_Fragment);

  // Link the program
  INFO("Linking program");
  GLuint ProgramID = gl->CreateProgram();

  if (vertex_file_path != NULL) gl->AttachShader(ProgramID, vertexShader.program);
  if (fragment_file_path != NULL) gl->AttachShader(ProgramID, fragmentShader.program);

  gl->LinkProgram(ProgramID);

  // Check the program
  gl->GetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  gl->GetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    gl->GetShaderInfoLog(ProgramID, InfoLogLength, NULL, msg);
    ERROR("Compiling shader: " TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET, msg);
    free(msg);
  }

  gl->DetachShader(ProgramID, vertexShader.program);
  gl->DetachShader(ProgramID, fragmentShader.program);

  //gl->DeleteShader(vertexShader.program);
  //gl->DeleteShader(fragmentShader.program);

  return ProgramID;
}

/* Returns a shader program */
Shader compose_shader(Shader *shaders, usize shaders_len) {
  const GladGLContext* gl = GLOBAL_PLATFORM->window->context;
  GLint Result = GL_FALSE;

  if (shaders_len == 0) {
    ERROR("No shaders provided!");
    return (Shader){.program = 0, .type = Shader_Error};
  }

  u32 prog = gl->CreateProgram();

  if (prog == 0) {
    ERROR("Failed to create program!");
    return (Shader){.program = 0, .type = Shader_Error};
  }

  for (int i = 0; i < shaders_len; i++) {
    gl->AttachShader(prog, shaders[i].program);
    INFO("Attaching shader %d to %d", shaders[i].program, prog);
  }

  gl->LinkProgram(prog);

  // Check the program
  gl->GetProgramiv(prog, GL_LINK_STATUS, &Result);
  if (Result != GL_TRUE) {
    // Get the size of the log
    int log_len = 0;
    int msg_len = 0;
    gl->GetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_len);
    char* msg = calloc(log_len + 1, sizeof(char));

    // Copy the log message(s)
    gl->GetProgramInfoLog(prog, log_len, &msg_len, msg);

    ERROR("(Compose) Compiling shader:\n" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET "\n", msg);
    free(msg);
  }

  for (int i = 0; i < shaders_len; i++) {
    gl->DetachShader(prog, shaders[i].program);
  }

  return (Shader){.program = prog, .type = Shader_Program};
}

void shaders_delete(Shader* shader, isize shader_len) {
  const GladGLContext* gl = GLOBAL_PLATFORM->window->context;

  for (isize i = 0; i < shader_len; i++) {
    gl->DeleteShader(shader[i].program);
  }
}

RenderObject RenderObject_new(float* model, Shader* shader, usize sz, float* uv, usize uv_sz, float* normal, usize normal_sz, u32 texture) {
  GladGLContext *gl = GLOBAL_PLATFORM->window->context;
  RenderObject o;

  gl->GenVertexArrays(1, &(o.vao));
  gl->BindVertexArray(o.vao);

  /* For each buffer in the shader, */
  /* The shader should be generalied, */
  gl->GenBuffers(1, &(o.vbo));
  gl->BindBuffer(GL_ARRAY_BUFFER, o.vbo);
  gl->BufferData(GL_ARRAY_BUFFER, sz, model, GL_STATIC_DRAW);

  gl->GenBuffers(1, &(o.col));
  gl->BindBuffer(GL_ARRAY_BUFFER, o.col);
  gl->BufferData(GL_ARRAY_BUFFER, uv_sz, uv, GL_STATIC_DRAW);

  gl->GenBuffers(1, &(o.normal));
  gl->BindBuffer(GL_ARRAY_BUFFER, o.normal);
  gl->BufferData(GL_ARRAY_BUFFER, normal_sz, normal, GL_STATIC_DRAW);

  o.shader = *shader;

  o.texture = texture;

  return o;
}

ShaderType guess_shadertype_from_filename(const char *restrict fname) {
  u32 stype = 0;
  const usize path_len = strlen(fname);

  if (path_len <= 4) {
    ERROR("Unable to determine shader type from suffix! (%s)", fname);
    return Shader_Error;
  }

  if (!strncmp(".vert", &fname[path_len - 5], 5)) { return Shader_Vertex; }
  if (!strncmp(".frag", &fname[path_len - 5], 5)) { return Shader_Fragment; }

  return Shader_Error;
}
