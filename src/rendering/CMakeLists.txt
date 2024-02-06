add_library(daw_rendering
  src/gl.c
  src/rendering.c
  src/text.c
  src/window.c
  ${GLAD_HEADER}
  )

add_custom_command(
  OUTPUT ${GLAD_HEADER}
  COMMAND
  glad
  --api gl:core=4.6
  --out-path ${CMAKE_BINARY_DIR}
  --reproducible
  c
  --header-only
  --mx
  )

set_property(SOURCE src/window.c APPEND PROPERTY OBJECT_DEPENDS ${GLAD_HEADER})
set_property(SOURCE src/rendering.c APPEND PROPERTY OBJECT_DEPENDS ${GLAD_HEADER})

target_include_directories(daw_rendering PRIVATE ${DAW_INCLUDE_DIRS})
target_link_libraries(daw_rendering PRIVATE
  OpenGL::GL
  cglm
  glfw
  )
