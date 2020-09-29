#ifndef STATE_H
#define STATE_H

#include <engine/memory.h>

typedef enum StateType {
  STATE_null,
#define State(name) STATE_##name,
#include <state_type_list.h>
#undef State
  STATE_quit,
} StateType;

extern const char *StateTypeStr[];

StateType(*State_updateFunc(StateType type))(void*);

void State_init(StateType type,   memory *mem);
void State_free(StateType type,   memory *mem);
StateType State_update(StateType type, memory *mem);

#endif
