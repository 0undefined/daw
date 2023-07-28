# Add the directory to the list of states.
# The directories contents will be compiled into a shared object file and linked
# to the main daw library
macro(daw_add_state STATEDIR)
  set_property(TARGET daw
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
    if(DAW_BUILD_DEBUG AND DAW_BUILD_HOTRELOAD)
      add_library(${STATEDIR} MODULE ${STATE_SOURCES})
    else()
      add_library(${STATEDIR} SHARED ${STATE_SOURCES})
    endif()
  else()
    add_library(${STATEDIR} OBJECT ${STATE_SOURCES})
  endif()

  target_include_directories(${STATEDIR} PUBLIC
    state_${STATEDIR}/include
    ${daw_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    include
    )

  set_property(TARGET daw
    APPEND PROPERTY LINK_LIBRARIES
    ${STATEDIR})

  list(APPEND STATE_LIST ${STATEDIR})
endmacro()
