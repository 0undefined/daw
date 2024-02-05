add_library(daw_utils
  src/btree.c
  src/fov.c
  src/hashmap.c
  src/misc.c
  src/stack.c
  src/vector.c
  )

target_include_directories(daw_utils PRIVATE include)
