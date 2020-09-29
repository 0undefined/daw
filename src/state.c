#include <engine/state.h>

#include <include_states.h>

typedef StateType state_update_t (void*);

const char *StateTypeStr[] = {
  "null",
#define State(name) #name,
#include <state_type_list.h>
#undef State
  "quit",
};

void State_init(StateType type,   memory *mem) {
  switch (type) {
#define State(name)                                   \
    case (STATE_##name): {                            \
      name##_init(memory_allocate(mem, sizeof(name##_state)));  \
      break;                                          \
    }
#include <state_type_list.h>
#undef State
    case STATE_null:
    case STATE_quit:
      _DEBUG("Got %s state.\n", StateTypeStr[type]);
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
#include <state_type_list.h>
#undef State
    case STATE_null:
    case STATE_quit:
      _DEBUG("Got %s state.\n", StateTypeStr[type]);
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
#include <state_type_list.h>
#undef State
    case STATE_null:
    case STATE_quit:
      return NULL; //_DEBUG("Got %s state.\n", StateTypeStr[type]);
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
#include <state_type_list.h>
#undef State
    case STATE_null:
    case STATE_quit:
      _DEBUG("Got %s state.\n", StateTypeStr[type]);
      break;
    default:
      exit(EXIT_FAILURE);
  }
  return next_state;
}
