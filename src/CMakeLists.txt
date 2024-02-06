set(DAW_INCLUDE_DIRS
  ${CMAKE_CURRENT_LIST_DIR}/core/include
  ${CMAKE_CURRENT_LIST_DIR}/ctrl/include
  ${CMAKE_CURRENT_LIST_DIR}/rendering/include
  ${CMAKE_CURRENT_LIST_DIR}/resources/include
  ${CMAKE_CURRENT_LIST_DIR}/ui/include
  ${CMAKE_CURRENT_LIST_DIR}/utils/include
  ${CMAKE_BINARY_DIR}/include
  )

add_subdirectory(core)
add_subdirectory(ctrl)
add_subdirectory(rendering)
add_subdirectory(resources)
add_subdirectory(ui)
add_subdirectory(utils)
