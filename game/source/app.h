#define DEBUG_DRAW_collision_volumes (1<<0)
#define DEBUG_DRAW_hurt_volumes      (1<<1)

typedef struct App App;
struct App
{
    MemoryArena permanent_arena;
    MemoryArena state_arena;
    MemoryArena frame_arena;
    
    u32 frame_index;
    f32 delta_t;
    f32 render_w;
    f32 render_h;
    
    Input input;
    Audio audio;
    Renderer renderer;
    UI ui;
    
    b32 anti_aliasing;
    b32 shadows;
    
    Font main_font;
    Font editor_font;
    Texture art;
    Sound footstep_1;
    Sound footstep_2;
    Sound footstep_3;
    Sound swing_1;
    Sound open_1;
    Sound music_1;
    
    StateType state_type;
    StateType next_state_type;
    f32 state_change_transition;
    
    u32 debug_draw_flags;
};

global App *app = 0;
global Platform *platform = 0;