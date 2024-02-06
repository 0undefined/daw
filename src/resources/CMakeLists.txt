add_library(daw_resources
  src/fonts.c
  src/scripts.c
  src/textures.c
  )

target_include_directories(daw_resources PRIVATE ${DAW_INCLUDE_DIRS})
