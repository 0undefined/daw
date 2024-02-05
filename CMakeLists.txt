cmake_minimum_required(VERSION 3.24)
project(daw
  VERSION 0.0.1
  DESCRIPTION "A Simple Hobby Game Engine"
  LANGUAGES C)

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
include(FetchContent)

option(ASAN "Enable address sanitizer. Only enabled when DAW_BUILD_DEBUG=ON")
option(UBSAN "Enable undefined behaviour sanitizer. Only enabled when DAW_BUILD_DEBUG=ON")

cmake_dependent_option(DAW_BUILD_DEBUG
  "Compile daw engine with debugging features" ON
  "CMAKE_BUILD_TYPE STREQUAL Debug" OFF)

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
cmake_dependent_option(DAW_BUILD_TOOLS
  "Build tools to manipulate a daw project" ON
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  message("Enabling LTO")
  set(${CMAKE_INTERPROCEDURAL_OPTIMIZATION} TRUE)
endif()

if(DAW_BUILD_DEBUG AND NOT_SUBPROJECT)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(DAW_BUILD_UBSAN AND DAW_BUILD_ASAN)
  message(FATAL_ERROR "You cannot build both asan and ubsan")
endif()

if(DAW_BUILD_DEBUG)
  set(BUILD_OPTS -Og -ggdb3 -fno-omit-frame-pointer)
else()
  set(BUILD_OPTS -O2 -flto=auto -fuse-linker-plugin -ffat-lto-objects
    -funroll-loops -ffast-math -fno-signed-zeros -fno-trapping-math
    -ffunction-sections -fdata-sections)
endif()

## Add some information before project configuration
# Git Sha1
execute_process(COMMAND
  ${GIT_EXECUTABLE} describe --match=NeVeRmAtCh --always --abbrev=6 --dirty
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_SHA
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

## Versioning
string(TIMESTAMP COMPILATION_DATE "%F")

set(ENGINE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(ENGINE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(ENGINE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(ENGINE_VERSION_TWEAK ${GIT_SHA})
string(TOLOWER "${CMAKE_BUILD_TYPE}" ENGINE_BUILD_TYPE)

## Packages
set(GLFW_USE_WAYLAND ON)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
FetchContent_Declare(cglm
  GIT_REPOSITORY https://github.com/recp/cglm.git
  GIT_TAG v0.9.2
)
FetchContent_MakeAvailable(cglm)

## Compilation information
add_subdirectory(src)

add_compile_options(${RELEASE_OPTS})
add_link_options(${RELEASE_OPTS})

add_library(${PROJECT_NAME}
  ${CMAKE_BINARY_DIR}/include/glad/gl.h
  #${ENGINE_SOURCES}
  )

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

target_include_directories(${PROJECT_NAME} PUBLIC
  include
  ${CMAKE_BINARY_DIR}/include
  ${ENGINE_INCLUDE})

target_link_libraries(${PROJECT_NAME}
  glfw
  OpenGL::GL
  cglm
  daw_core
  daw_rendering
  daw_ctrl
  daw_resources
  daw_ui
  daw_utils
  NOT:$<PLATFORM_ID:Windows>>:m>
  $<$<BOOL:${DAW_BUILD_HOTRELOAD}>:dl>
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<BOOL:${DAW_BUILD_UBSAN}>>:ubsan>
)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

if(DAW_BUILD_ASAN AND NOT C_COMPILER_ID STREQUAL MSVC)
  add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize-address-use-after-scope)
  add_link_options(-fsanitize=address -fsanitize=leak -fsanitize-address-use-after-scope)
endif()

if(DAW_BUILD_UBSAN AND NOT C_COMPILER_ID STREQUAL MSVC)
  add_compile_options(-fsanitize=undefined -fsanitize-undefined-trap-on-error -fno-sanitize-recover)
  add_link_options(-fsanitize=undefined -fsanitize-undefined-trap-on-error -fno-sanitize-recover)
endif()

target_compile_options(${PROJECT_NAME} PUBLIC
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra>
  # Dont be pedantic when using hot reloading.
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<NOT:$<BOOL:${DAW_BUILD_HOTRELOAD}>>>:-pedantic>
  $<$<C_COMPILER_ID:MSVC>:/W4>
  # Debug related flags.  sorry windows, you're just not that important.
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  $<$<BOOL:${DAW_BUILD_DEBUG}>:_DEBUG>
  $<$<BOOL:${DAW_BUILD_HOTRELOAD}>:DAW_BUILD_HOTRELOAD>
)

configure_file(${CMAKE_CURRENT_LIST_DIR}/tools/cmake/configure.h.in
  ${CMAKE_BINARY_DIR}/include/engine/configure.h)

configure_file(${CMAKE_CURRENT_LIST_DIR}/tools/cmake/list_of_states.h.in
  ${CMAKE_BINARY_DIR}/include/states/list_of_states.h)

configure_file(${CMAKE_CURRENT_LIST_DIR}/tools/cmake/all_states.h.in
  ${CMAKE_BINARY_DIR}/include/states/all_states.h)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/tools/cmake")
if (NOT NOT_SUBPROJECT)
  set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" PARENT_SCOPE)
endif()
include(DawAddState)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/include/glad/gl.h
  COMMAND
  glad
  --api gl:core=3.3
  --out-path ${CMAKE_BINARY_DIR}
  --reproducible
  c
  --header-only
  --mx
)

message("Configured ${PROJECT_NAME} ${PROJECT_VERSION}")
message("version:          ${ENGINE_VERSION_MAJOR}.${ENGINE_VERSION_MINOR}.${ENGINE_VERSION_PATCH}-${ENGINE_VERSION_TWEAK} (${ENGINE_BUILD_TYPE} build)")
message("enable debug:     ${DAW_BUILD_DEBUG}")
message("enable hotreload: ${DAW_BUILD_HOTRELOAD}")
message("enable asan:      ${DAW_BUILD_ASAN}")
message("enable ubsan:     ${DAW_BUILD_UBSAN}")
message("build tools:      ${DAW_BUILD_TOOLS}")
