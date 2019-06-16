
typedef enum StateType StateType;
enum StateType
{
    STATE_invalid,
#define State(name, lower_name) STATE_##lower_name,
#include "state_type_list.h"
};

internal void StateInit(StateType type, MemoryArena *state_arena);
internal void StateCleanUp(StateType type, MemoryArena *state_arena);
internal StateType StateUpdate(StateType type, MemoryArena *state_arena);