#include <string.h>

#include <engine/state.h>
#include <engine/logging.h>
#include <engine/dltools.h>
#include <engine/input.h>

typedef StateType state_update_t(void*);

const char *StateTypeStr[] = {
  "null",
#define State(name) #name,
#include <states/list_of_states.h>
#undef State
  "quit",
};

// Setup API for states
#define State(name)                                  \
typedef struct name##_state name##_state;            \
typedef void (state_##name##_init_t)(name##_state*); \
typedef void (state_##name##_free_t)(name##_state*); \
typedef StateType (state_##name##_update_t)(name##_state*);
#include <states/list_of_states.h>
#undef State

#ifdef DAW_BUILD_HOTRELOAD

// When hotreloading is enabled, we want to assign state function pointers
// dynamically.
#define State(name)                                      \
state_##name##_init_t   *name##_init = NULL;             \
state_##name##_free_t   *name##_free = NULL;             \
state_##name##_update_t *name##_update = NULL;           \
																												 \
void* libstate_##name = NULL;                            \
const char* libstate_##name##_str = "lib" #name ".so";
#else

// Otherwise we just declare them.
#define State(name)                        \
extern state_##name##_init_t   name##_init;       \
extern state_##name##_free_t   name##_free;       \
extern state_##name##_update_t name##_update;
#endif

#include <states/list_of_states.h>
#undef State

#include <states/all_states.h>

void binding_t_free(binding_t* b);

void State_init(StateType type,   memory *mem) {
  switch (type) {
#define State(name)                                   \
    case (STATE_##name): {                            \
      name##_init(memory_allocate(mem, sizeof(name##_state)));  \
      break;                                          \
    }
#include <states/list_of_states.h>
#undef State
    case STATE_null:
    case STATE_quit:
      DEBUG("Got %s state.\n", StateTypeStr[type]);
      break;
    default:
      exit(EXIT_FAILURE);
  }
}


void State_free(StateType type,   memory *mem) {
  switch (type) {
#define State(name)           \
    case (STATE_##name): {    \
      name##_free(mem->data); \
      break;                  \
    }
#include <states/list_of_states.h>
#undef State
    case STATE_null:
    case STATE_quit:
      DEBUG("Got %s state.\n", StateTypeStr[type]);
      break;
    default:
      exit(EXIT_FAILURE);
  }
  memory_clear(mem);
}


StateType (*State_updateFunc(StateType type))(void*) {
  switch (type) {
#ifdef DAW_BUILD_HOTRELOAD
#define State(name)                          \
    case (STATE_##name): {                   \
    return  (state_update_t*)name##_update;  \
      break;                                 \
    }
#else
#define State(name)                          \
    case (STATE_##name): {                   \
    return  (state_update_t*)&name##_update; \
      break;                                 \
    }
#endif
#include <states/list_of_states.h>
#undef State
    case STATE_null:
    case STATE_quit:
      return NULL; //DEBUG("Got %s state.\n", StateTypeStr[type]);
      break;
    default:
      exit(EXIT_FAILURE);
  }
  return NULL;
}

StateType State_update(StateType type, memory *mem) {
  StateType next_state = STATE_null;
  switch (type) {
#define State(name)                          \
    case (STATE_##name): {                   \
      next_state = name##_update(mem->data); \
      break;                                 \
    }
#include <states/list_of_states.h>
#undef State
    case STATE_null:
    case STATE_quit:
      DEBUG("Got %s state.\n", StateTypeStr[type]);
      break;
    default:
      exit(EXIT_FAILURE);
  }
  return next_state;
}

#ifdef DAW_BUILD_HOTRELOAD
bool State_reload(StateType type, i_ctx **ctx, usize ctx_len) {
	void* libptr = NULL;

  switch (type) {
#define State(name)                                                             \
    case (STATE_##name): {                                                      \
      if (libstate_##name == NULL) {                                            \
        libstate_##name = dynamic_library_open(libstate_##name##_str);          \
      } else {                                                                  \
        libstate_##name =                                                       \
          dynamic_library_reload(libstate_##name, libstate_##name##_str);       \
      }                                                                         \
      if (libstate_##name == NULL) {                                            \
        ERROR("Failed loading shared object: %s (%s)",                          \
            libstate_##name##_str,                                              \
            dynamic_library_get_error());                                       \
        return false;                                                           \
      }                                                                         \
                                                                                \
      name##_init = (state_##name##_init_t*)dynamic_library_get_symbol(libstate_##name, STR( name##_init ) ); \
      name##_free = (state_##name##_free_t*)dynamic_library_get_symbol(libstate_##name, STR( name##_free ) ); \
      name##_update = (state_##name##_update_t*)dynamic_library_get_symbol(libstate_##name, STR( name##_update ) ); \
      libptr = libstate_##name;                                                 \
      break;                                                                    \
    }
#include <states/list_of_states.h>
#undef State
    case STATE_null:
    case STATE_quit:
      ERROR("Invalid state");
      DEBUG("Got %s state.\n", StateTypeStr[type]);
      return false;
    default:
      exit(EXIT_FAILURE);
  }
	return state_refresh_input_ctx(libptr, ctx, ctx_len);
}
#endif
