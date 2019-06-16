
internal void
StateInit(StateType type, MemoryArena *state_arena)
{
    switch(type)
    {
#define State(name, lower_name) case STATE_##lower_name: { name##StateInit(MemoryArenaAllocate(state_arena, sizeof(name##State))); break; }
#include "state_type_list.h"
        default: INVALID_CODE_PATH;
    }
}

internal void
StateCleanUp(StateType type, MemoryArena *state_arena)
{
    switch(type)
    {
#define State(name, lower_name) case STATE_##lower_name: { name##StateCleanUp(state_arena->memory); break; }
#include "state_type_list.h"
        default: INVALID_CODE_PATH;
    }
    MemoryArenaClear(state_arena);
}

internal StateType
StateUpdate(StateType type, MemoryArena *state_arena)
{
    StateType next_state = STATE_invalid;
    switch(type)
    {
#define State(name, lower_name) case STATE_##lower_name: { next_state = name##StateUpdate(state_arena->memory); break; }
#include "state_type_list.h"
        default: INVALID_CODE_PATH;
    }
    return next_state;
}