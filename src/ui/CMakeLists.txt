add_library(daw_ui
  src/positioning.c
  src/rendering.c
  )

target_include_directories(daw_ui PRIVATE ${DAW_INCLUDE_DIRS})
