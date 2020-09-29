cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
else()
  set(NOT_SUBPROJECT OFF)
endif()

option(ENGINE_DEBUG "Compile engine with debugging features" OFF)
option(ENGINE_HOT_RELOADING "Compile with hot-reloading enabled (only works with debug mode)" ON)

set(ENGINE_SOURCES
  src/btree.c
  src/engine.c
  src/fov.c
  src/hashmap.c
  src/logging.c
  src/memory.c
  src/rendering.c
  src/stack.c
  src/state.c
  src/ui_positioning.c
  src/ui_rendering.c
  src/utils.c
  src/vector.c
)

add_library(engine ${ENGINE_SOURCES})

target_include_directories(engine PUBLIC
  include
  ${CMAKE_BINARY_DIR}/include
  ${ENGINE_INCLUDE})
target_link_libraries(engine SDL2 SDL2_image SDL2_ttf m)
target_compile_features(engine PRIVATE c_std_99)

configure_file(${CMAKE_CURRENT_LIST_DIR}/state_type_list.h.in
  ${CMAKE_BINARY_DIR}/include/state_type_list.h)

configure_file(${CMAKE_CURRENT_LIST_DIR}/include_states.h.in
  ${CMAKE_BINARY_DIR}/include/include_states.h)


# Add the directory to the list of states
# The directorys contents will be compiled into a shared object file and linked
# to the main executable
macro(add_state STATEDIR)
  if(NOT_SUBPROJECT)
  else()
    set_property(TARGET engine
      APPEND PROPERTY INCLUDE_DIRECTORIES
      ${CMAKE_SOURCE_DIR}/state_${STATEDIR}/include)

    file(APPEND ${CMAKE_BINARY_DIR}/include/state_type_list.h
      "State(${STATEDIR})\n")

    file(APPEND ${CMAKE_BINARY_DIR}/include/include_states.h
      "#include <states/${STATEDIR}.h>\n")

    file(GLOB STATE_SOURCES
      LIST_DIRECTORIES false
      state_${STATEDIR}/src/*.c
    )

    # TODO: When state reloading is implemented properly, add MODULE library
    # option In general, this should only be available when debugging.
    if(BUILD_SHARED_LIBS)
      add_library(${STATEDIR} SHARED ${STATE_SOURCES})
    else()
      add_library(${STATEDIR} OBJECT ${STATE_SOURCES})
    endif()

    target_include_directories(${STATEDIR} PUBLIC
      state_${STATEDIR}/include
      engine/include
      ${CMAKE_BINARY_DIR}/include
      include
      )

    set_property(TARGET engine
      APPEND PROPERTY LINK_LIBRARIES
      ${STATEDIR})

  endif()

  list(APPEND STATE_LIST ${STATEDIR})
  #list(APPEND ENGINE_INCLUDE state_${STATEDIR}/include)
endmacro()
