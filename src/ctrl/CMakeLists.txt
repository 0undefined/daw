add_library(daw_ctrl
  src/controller.c
  src/ctx.c
  src/input.c
  src/keyboard.c
  src/mouse.c
  )

target_include_directories(daw_ctrl PRIVATE include)
