add_library(daw_core
  src/cleanup.c
  src/dltools.c
  src/harness.c
  src/init.c
  src/logging.c
  src/loop.c
  src/memory.c
  src/state.c
  )

target_include_directories(daw_core PRIVATE include)
