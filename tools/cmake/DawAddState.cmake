# Add the directory to the list of states.
# The directories contents will be compiled into a shared object file and linked
# to the main daw library, unless you compile statically, or link during runtime.
#
# Say you want to add a new state, called ${STATENAME} to your project, located
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
  set_property(TARGET daw_core
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

  file(GLOB DAW_INCLUDE_DIRS
    LIST_DIRECTORIES true
    ${daw_SOURCE_DIR}/src/*/include
  )

  # TODO: When state reloading is implemented properly, add MODULE library
  # option. In general, this should only be available when debugging.
  if(BUILD_SHARED_LIBS OR DAW_BUILD_HOTRELOAD)
    if(DAW_BUILD_HOTRELOAD)
      add_library(${STATENAME} MODULE ${STATE_SOURCES})
    else()
      add_library(${STATENAME} SHARED ${STATE_SOURCES})
    endif()

    target_link_libraries(${STATENAME} PUBLIC daw)

    set_property(TARGET ${STATENAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
  else()
    add_library(${STATENAME} OBJECT ${STATE_SOURCES})

    # The game-state source is withing the core module
    set_property(TARGET daw_core
      APPEND PROPERTY INCLUDE_DIRECTORIES
      ${CMAKE_SOURCE_DIR}/state_${STATENAME}/include)
    set_property(TARGET daw_core
      APPEND PROPERTY LINK_LIBRARIES
      ${STATENAME})
  endif()

  target_include_directories(${STATENAME} PUBLIC
    state_${STATENAME}/include
    ${CMAKE_BINARY_DIR}/include
    ${DAW_INCLUDE_DIRS}
    include
    )

  if(NOT (DAW_BUILD_HOTRELOAD OR BUILD_SHARED_LIBS))
    set_property(TARGET daw
      APPEND PROPERTY LINK_LIBRARIES
      ${STATENAME})
  endif()

  list(APPEND STATE_LIST ${STATENAME})
endmacro()
