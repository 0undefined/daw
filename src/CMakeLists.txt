# Set global include dirs
set(DAW_INCLUDE_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/core/include
  ${CMAKE_CURRENT_LIST_DIR}/ctrl/include
  ${CMAKE_CURRENT_LIST_DIR}/rendering/include
  ${CMAKE_CURRENT_LIST_DIR}/resources/include
  ${CMAKE_CURRENT_LIST_DIR}/utils/include
  ${CMAKE_BINARY_DIR}/include
  )

# Add the engine modules
add_subdirectory(core)
add_subdirectory(ctrl)
add_subdirectory(rendering)
add_subdirectory(resources)
add_subdirectory(utils)

# DAW Engine compilation spec
add_library(${PROJECT_NAME}
  ${GLAD_HEADER}
  )

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)

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
  daw_utils
  $<$<NOT:$<PLATFORM_ID:Windows>>:m>
  $<$<BOOL:${DAW_BUILD_HOTRELOAD}>:dl>
  $<$<AND:$<NOT:$<C_COMPILER_ID:MSVC>>,$<BOOL:${DAW_BUILD_UBSAN}>>:ubsan>
)

target_compile_features(${PROJECT_NAME} PRIVATE c_std_99)

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
