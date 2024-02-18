#ifndef STATE_H
#define STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <engine/core/memory.h>

typedef enum StateType {
  STATE_null,
#define State(name) STATE_##name,
#include <states/list_of_states.h>
#undef State
  STATE_quit,
} StateType;

extern const char* StateTypeStr[];

StateType (*State_updateFunc(StateType type))(f64, void*);

void State_init(StateType type, memory* mem, void* arg);
void* State_free(StateType type, memory* mem);
StateType State_update(StateType type, f64 dt, memory* mem);

/* Reloads shared object file associated with state */
#ifdef DAW_BUILD_HOTRELOAD
#include <engine/ctrl/input.h>
bool State_reload(StateType type, i_ctx** ctx, usize ctx_len);
bool state_refresh_input_ctx(void* lib, i_ctx** ctx, usize ctx_len);

#else
#define State_reload(_, _0, _1) true
#define state_refresh_input_ctx(_0, _1, _2) true

#endif

#ifdef __cplusplus
}
#endif
#endif
