#include <stdlib.h>
#include <stdio.h>

#include <glad/gl.h>

#include <engine/core/types.h>
#include <engine/core/logging.h>

// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
GLuint LoadShaders(
    const GladGLContext* gl,
    const char * vertex_file_path,
    const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = gl->CreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = gl->CreateShader(GL_FRAGMENT_SHADER);

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
  gl->ShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  gl->CompileShader(VertexShaderID);

  // Check Vertex Shader
  gl->GetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  gl->GetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ) {
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    gl->GetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, msg);
    ERROR("Compiling shader: %s\n", msg);
    free(msg);
  }

  // Compile Fragment Shader
  INFO("Compiling shader: %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode;
  gl->ShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  gl->CompileShader(FragmentShaderID);

  // Check Fragment Shader
  gl->GetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  gl->GetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    gl->GetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, msg);
    ERROR("Compiling shader: %s\n", msg);
    free(msg);
  }

  // Link the program
  INFO("Linking program\n");
  GLuint ProgramID = gl->CreateProgram();
  gl->AttachShader(ProgramID, VertexShaderID);
  gl->AttachShader(ProgramID, FragmentShaderID);
  gl->LinkProgram(ProgramID);

  // Check the program
  gl->GetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  gl->GetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if ( InfoLogLength > 0 ){
    char* msg = calloc(InfoLogLength + 1, sizeof(char));
    gl->GetShaderInfoLog(ProgramID, InfoLogLength, NULL, msg);
    ERROR("Compiling shader: %s\n", msg);
    free(msg);
  }

  gl->DetachShader(ProgramID, VertexShaderID);
  gl->DetachShader(ProgramID, FragmentShaderID);

  gl->DeleteShader(VertexShaderID);
  gl->DeleteShader(FragmentShaderID);

  free(VertexShaderCode);
  free(FragmentShaderCode);
  return ProgramID;
}
