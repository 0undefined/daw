# Add the directory to the list of states.
# The directories contents will be compiled into a shared object file and linked
# to the main daw library, unless you compile statically, or link during runtime.
#
# Say you want to add a new state, called ${STATENAME} to your project, localted
# at ${PROJECT_ROOT}, Then your directory structure for your main project should
# be as follows:
#     ${PROJECT_ROOT}/
#     ├ state_${STATENAME}/
#     │ ├ includes/states/${STATENAME}.h
#     │ └ src/ ..
#     ├ state_foo/ ..
#     └ state_bar/ ..
#
# Then call `daw_add_state(${STATENAME}) for each of your states.
macro(daw_add_state STATENAME)
  # Add state include directory to the engines target
  set_property(TARGET daw
    APPEND PROPERTY INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/state_${STATENAME}/include)

  # Append state to list of states
  file(APPEND ${CMAKE_BINARY_DIR}/include/states/list_of_states.h
    "State(${STATENAME})\n")

  # Append header inclusion of state to common state header
  file(APPEND ${CMAKE_BINARY_DIR}/include/states/all_states.h
    "#include <states/${STATENAME}.h>\n")

  # Glob the states sources, not my proudest moment
  file(GLOB STATE_SOURCES
    LIST_DIRECTORIES false
    state_${STATENAME}/src/*.c
  )

  # TODO: When state reloading is implemented properly, add MODULE library
  # option In general, this should only be available when debugging.
  if(BUILD_SHARED_LIBS)
    if(DAW_BUILD_DEBUG AND DAW_BUILD_HOTRELOAD)
      add_library(${STATENAME} MODULE ${STATE_SOURCES})
      target_compile_definitions(${STATENAME} PUBLIC
        $<$<BOOL:${DAW_BUILD_DEBUG}>:_DEBUG>
      )
    else()
      add_library(${STATENAME} SHARED ${STATE_SOURCES})
    endif()

    set_property(TARGET ${STATENAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

  else()
    add_library(${STATENAME} OBJECT ${STATE_SOURCES})
  endif()

  target_include_directories(${STATENAME} PUBLIC
    state_${STATENAME}/include
    ${daw_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    include
    )

  if(NOT DAW_BUILD_HOTRELOAD)
    set_property(TARGET daw
      APPEND PROPERTY LINK_LIBRARIES
      ${STATENAME})
  endif()

  list(APPEND STATE_LIST ${STATENAME})
endmacro()
