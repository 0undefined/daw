cmake_minimum_required(VERSION 3.24)
project(daw VERSION 0.0.1 LANGUAGES C)

# We really don't want any in-source builds
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON CACHE BOOL "Prevents cmake -S. -B.")
set(CMAKE_DISABLE_SOURCE_CHANGES ON CACHE BOOL "Prevent writing files to CMAKE_SOURCE_DIR under configure")

# Check if we are used as a sub project/module or not
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(NOT_SUBPROJECT ON)
else()
  set(NOT_SUBPROJECT OFF)
endif()

include(CMakeDependentOption)

option(ASAN "Enable address sanitizer. Only enabled when DAW_BUILD_DEBUG=ON")
option(UBSAN "Enable undefined behaviour sanitizer. Only enabled when DAW_BUILD_DEBUG=ON")

cmake_dependent_option(DAW_BUILD_DEBUG
  "Compile daw engine with debugging features" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug" OFF)

# unused
cmake_dependent_option(DAW_BUILD_HOTRELOAD
  "Compile daw engine with hot reloading enabled" ON
  "NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME;CMAKE_BUILD_TYPE STREQUAL Debug;NOT WIN32;BUILD_SHARED_LIBS" OFF)

cmake_dependent_option(DAW_BUILD_ASAN
  "Compile daw engine with address sanitizer (asan)" ON
  "DAW_BUILD_DEBUG;ASAN" OFF)

cmake_dependent_option(DAW_BUILD_UBSAN
  "Compile daw engine with undefined behaviour sanitizer (ubsan)" ON
  "DAW_BUILD_DEBUG;UBSAN" OFF)

# unused
cmake_dependent_option(DAW_BUILD_STANDALONE
  # Would require us to enable dynamically registered states.
  "Compile daw as a standalone library. This is not implemented yet." OFF
  "TRUE" OFF)

# unused
cmake_dependent_option(DAW_BUILD_TOOLS
  "Build tools to manipulate a daw project" ON
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  message("Enabling LTO")
  set(${CMAKE_INTERPROCEDURAL_OPTIMIZATION} TRUE)
endif()

if(DAW_BUILD_UBSAN AND DAW_BUILD_ASAN)
  message(FATAL_ERROR "You cannot build both asan and ubsan")
endif()

set(ENGINE_SOURCES
  src/btree.c
  src/dltools.c
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

add_library(daw
  ${ENGINE_SOURCES})

target_include_directories(daw PUBLIC
  include ${CMAKE_BINARY_DIR}/include ${ENGINE_INCLUDE})

target_link_libraries(${PROJECT_NAME}
  SDL2 SDL2_image SDL2_ttf
  $<$<NOT:$<PLATFORM_ID:Windows>>:m>
  $<$<BOOL:${DAW_BUILD_HOTRELOAD}>:dl>
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<BOOL:${DAW_BUILD_UBSAN}>>:ubsan>
)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

target_compile_options(${PROJECT_NAME} PUBLIC
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra>
  # Dont be pedantic when using hot reloading.
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<NOT:$<BOOL:${DAW_BUILD_HOTRELOAD}>>>:-pedantic>
  $<$<C_COMPILER_ID:MSVC>:/W4>
  # Debug related flags.  sorry windows, you're just not that important.
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<BOOL:${DAW_BUILD_DEBUG}>>:-Og -ggdb3 -fno-omit-frame-pointer>
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<BOOL:${DAW_BUILD_ASAN}>>:-fsanitize=address -fsanitize=leak -fsanitize-address-use-after-scope>
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<BOOL:${DAW_BUILD_UBSAN}>>:-fsanitize=undefined -fsanitize-undefined-trap-on-error -fno-sanitize-recover>

  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<NOT:$<BOOL:${DAW_BUILD_DEBUG}>>>:-O2 -flto=auto -fuse-linker-plugin -ffat-lto-objects -funroll-loops -ffast-math -fno-signed-zeros -fno-trapping-math -ffunction-sections -fdata-sections>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  $<$<BOOL:${DAW_BUILD_DEBUG}>:DAW_BUILD_DEBUG>
  $<$<BOOL:${DAW_BUILD_HOTRELOAD}>:DAW_BUILD_HOTRELOAD>
)


configure_file(${CMAKE_CURRENT_LIST_DIR}/tools/cmake/list_of_states.h.in
  ${CMAKE_BINARY_DIR}/include/states/list_of_states.h.in)

configure_file(${CMAKE_CURRENT_LIST_DIR}/tools/cmake/all_states.h.in
  ${CMAKE_BINARY_DIR}/include/states/all_states.h)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools/cmake")
if (NOT NOT_SUBPROJECT)
  set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endif()
include(DawAddState)

message("Configured ${PROJECT_NAME} ${PROJECT_VERSION}")
message("Build type:       ${CMAKE_BUILD_TYPE}")
message("enable debug:     ${DAW_BUILD_DEBUG}")
message("enable hotreload: ${DAW_BUILD_HOTRELOAD}")
message("enable asan:      ${DAW_BUILD_ASAN}")
message("enable ubsan:     ${DAW_BUILD_UBSAN}")
message("build tools:      ${DAW_BUILD_TOOLS}")
