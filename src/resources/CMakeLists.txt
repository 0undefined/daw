## Uncomment the following to fetch and build assimp.
#FetchContent_Declare(assimp
#  GIT_REPOSITORY https://github.com/assimp/assimp.git
#  GIT_TAG v5.3.1
#)
#FetchContent_MakeAvailable(assimp)

add_library(daw_resources
  src/resources.c
  src/fonts.c
  src/scripts.c
  src/textures.c
  src/model.c
  )

target_include_directories(daw_resources PRIVATE ${DAW_INCLUDE_DIRS})
target_link_libraries(daw_resources PRIVATE
  assimp
  cglm
  )
