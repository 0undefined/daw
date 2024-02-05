#ifndef STATE_H
#define STATE_H

#include <engine/memory.h>

typedef enum StateType {
  STATE_null,
#define State(name) STATE_##name,
#include <states/list_of_states.h>
#undef State
  STATE_quit,
} StateType;

extern const char* StateTypeStr[];

StateType (*State_updateFunc(StateType type))(void*);

void State_init(StateType type, memory* mem);
void State_free(StateType type, memory* mem);
StateType State_update(StateType type, memory* mem);

/* Reloads shared object file associated with state */
#ifdef DAW_BUILD_HOTRELOAD
#include <engine/input.h>
bool State_reload(StateType type, i_ctx** ctx, usize ctx_len);

#else
#define State_reload(_, _0, _1) true
#endif

#endif
