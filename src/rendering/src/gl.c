#include <stdlib.h>
#include <stdio.h>

#include <glad/gl.h>

#include <engine/core/types.h>
#include <engine/core/logging.h>

#include <engine/rendering/rendering.h>
#include <engine/core/platform.h>

extern Platform* GLOBAL_PLATFORM;

isize f_get_sz(FILE* f) {
  if (f == NULL) {
    ERROR("File was null!");
    return 0;
  }

  const isize pos = ftell(f);

  fseek(f, 0, SEEK_END);
  const i64 size = ftell(f);

  // Reset the position to the position prior to calling f_get_sz
  fseek(f, pos, SEEK_SET);

  return size;
}

const GLuint
compile_shader(const GladGLContext *gl, const char* file_path, const GLenum shader_type) {
  GLuint shaderID = 0;

  if (file_path == NULL) {
    WARN("Empty path to shader");
    return (GLuint)0;
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  char* source;
  FILE* file = fopen(file_path, "r");

  shaderID = gl->CreateShader(shader_type);

  if(file != NULL) {
    const i64 size = f_get_sz(file);

    source = calloc(size + 1, sizeof(char));

    // Assume the whole file is successfully read
    fread(source, sizeof(char), size, file);

    fclose(file);
  } else {
    ERROR("Cannot open \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET"\".", file_path);
    return 0;
  }

  // Compile shader
  INFO("Compiling shader \"" TERM_COLOR_YELLOW "%s" TERM_COLOR_RESET"\".", file_path);
  char const * src_ptr = source;
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
  free(source);

  return shaderID;
}


// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
GLuint LoadShaders(
    const GladGLContext* gl,
    const char * vertex_file_path,
    const char * fragment_file_path) {

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Create the shaders
  const GLuint vertexShader = compile_shader(gl, vertex_file_path, GL_VERTEX_SHADER);
  const GLuint fragmentShader = compile_shader(gl, fragment_file_path, GL_FRAGMENT_SHADER);

  // Link the program
  INFO("Linking program");
  GLuint ProgramID = gl->CreateProgram();

  if (vertex_file_path != NULL) gl->AttachShader(ProgramID, vertexShader);
  if (fragment_file_path != NULL) gl->AttachShader(ProgramID, fragmentShader);

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

  gl->DetachShader(ProgramID, vertexShader);
  gl->DetachShader(ProgramID, fragmentShader);

  gl->DeleteShader(vertexShader);
  gl->DeleteShader(fragmentShader);

  return ProgramID;
}

RenderObject RenderObject_new(float* model, usize sz, float* uv, usize uv_sz) {
  GladGLContext *gl = GLOBAL_PLATFORM->window->context;
  RenderObject o;

  // TODO: implement index buffer!

  gl->GenVertexArrays(1, &(o.vao));
  gl->BindVertexArray(o.vao);

  gl->GenBuffers(1, &(o.vbo));
  gl->BindBuffer(GL_ARRAY_BUFFER, o.vbo);
  gl->BufferData(GL_ARRAY_BUFFER, sz, model, GL_STATIC_DRAW);

  gl->GenBuffers(1, &(o.col));
  gl->BindBuffer(GL_ARRAY_BUFFER, o.col);
  gl->BufferData(GL_ARRAY_BUFFER, uv_sz, uv, GL_STATIC_DRAW);
  //gl->GenBuffers(1, &(o.ibo));
  //gl->BindBuffer(GL_ARRAY_BUFFER, o.ibo);
  //gl->BufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

  o.shader.program = LoadShaders(gl, "shader.vert", "shader.frag");

  return o;
}
