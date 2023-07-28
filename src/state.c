#include <engine/state.h>
#include <engine/logging.h>

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
#include <states/all_states.h>
#undef State

#ifdef DAW_BUILD_HOTRELOAD
// When hotreloading is enabled, we want to assign state function pointers
// dynamically.
#define State(name)                             \
state_##name##_init_t   *name##_init = NULL;    \
state_##name##_free_t   *name##_free = NULL;    \
state_##name##_update_t *name##_update = NULL;

#else
// Otherwise we just declare them.
#define State(name)                        \
state_##name##_init_t   name##_init;       \
state_##name##_free_t   name##_free;       \
state_##name##_update_t name##_update;
#endif

#include <states/all_states.h>
#undef State

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
#define State(name)                          \
    case (STATE_##name): {                   \
    return  (state_update_t*)&name##_update; \
      break;                                 \
    }
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
