add_library(daw_rendering
  src/gl.c
  src/rendering.c
  src/text.c
  src/window.c
  )

target_include_directories(daw_rendering PRIVATE include)
target_link_libraries(daw_rendering PRIVATE
  OpenGL::GL
  cglm
  glfw
  )
