add_library(daw_core
  src/cleanup.c
  src/dltools.c
  src/harness.c
  src/init.c
  src/logging.c
  src/loop.c
  src/memory.c
  src/state.c
  src/thread.c
  )

set_property(SOURCE src/loop.c APPEND PROPERTY OBJECT_DEPENDS ${GLAD_HEADER})

target_include_directories(daw_core PRIVATE ${DAW_INCLUDE_DIRS})
target_link_libraries(daw_core PRIVATE cglm)
