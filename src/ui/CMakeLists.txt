add_library(daw_ui
  src/positioning.c
  src/rendering.c
  )

target_include_directories(daw_ui PRIVATE ${DAW_INCLUDE_DIRS})
target_link_libraries(daw_ui PRIVATE
  OpenGL::GL
  cglm
  )
