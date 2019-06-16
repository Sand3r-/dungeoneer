#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "ext/stb_image.h"
#include "ext/stb_vorbis.c"

#include "constants.h"
#include "damage.h"
#include "map_tile.h"
#include "debug.h"
#include "utilities.h"
#include "assets.h"
#include "generated/generated_structs.h"
#include "input.h"
#include "renderer.h"
#include "audio.h"
#include "ui.h"
#include "settings_menu.h"
#include "entity_id.h"
#include "item.h"
#include "generated/generated_entity.h"
#include "map.h"
#include "component.h"
#include "state.h"
#include "app.h"
#include "camera.h"
#include "particle.h"
#include "player.h"
#include "projectile.h"
#include "generated/generated_maps.h"

#include "debug.c"
#include "utilities.c"
#include "assets.c"
#include "renderer.c"
#include "audio.c"
#include "ui.c"
#include "settings_menu.c"
#include "map.c"
#include "component.c"
#include "state_title.c"
#include "state_game.c"
#include "state_game_over.c"
#include "state.c"
#include "particle.c"
#include "player.c"
#include "projectile.c"
#include "generated/generated_structs.c"
#include "generated/generated_entity.c"
#include "generated/generated_maps.c"

internal void
Update(Platform *platform_)
{
    platform = platform_;
    app = platform->permanent_storage;
    
    if(platform->initialized == 0)
    {
        platform->initialized = 1;
        app->permanent_arena = MemoryArenaInit(platform->permanent_storage,
                                               platform->permanent_storage_size);
        MemoryArenaAllocate(&app->permanent_arena, sizeof(App));
        
        app->frame_index = 0;
        
        u32 state_arena_size = app->permanent_arena.memory_left;
        app->state_arena = MemoryArenaInit(MemoryArenaAllocate(&app->permanent_arena, state_arena_size),
                                           state_arena_size);
        
        app->frame_arena = MemoryArenaInit(platform->scratch_storage,
                                           platform->scratch_storage_size);
        
        SeedRandomNumberGenerator();
        
        platform->target_frames_per_second = 60.f;
        AudioInit(&app->audio);
        RendererInit(&app->renderer);
        UIInit(&app->ui);
        
        app->main_font = FontLoad("data/font/main");
        app->editor_font = FontLoad("data/font/mono");
        app->art = TextureLoad("data/texture/art");
        app->footstep_1 = SoundLoad("data/sound/footstep_1");
        app->footstep_2 = SoundLoad("data/sound/footstep_2");
        app->footstep_3 = SoundLoad("data/sound/footstep_3");
        app->swing_1 = SoundLoad("data/sound/swing_1");
        app->open_1 = SoundLoad("data/sound/open_1");
        app->music_1 = SoundLoad("data/sound/music_1");
        
        app->state_type = STATE_game;
        app->next_state_type = STATE_invalid;
        app->state_change_transition = 1.f;
        StateInit(app->state_type, &app->state_arena);
    }
    else
    {
        ++app->frame_index;
    }
    
    // NOTE(rjf): Load per-frame platform data
    {
        InputBeginFrame(&app->input, platform);
        app->delta_t = 1.f / platform->target_frames_per_second;
        app->render_w = (f32)platform->window_width;
        app->render_h = (f32)platform->window_height;
    }
    
    // NOTE(rjf): Update
    MemoryArenaClear(&app->frame_arena);
    RendererBeginFrame(&app->renderer, app->render_w, app->render_h);
    {
        
        if(platform->key_pressed[KEY_f1])
        {
            app->debug_draw_flags ^= DEBUG_DRAW_collision_volumes;
        }
        else if(platform->key_pressed[KEY_f2])
        {
            app->debug_draw_flags ^= DEBUG_DRAW_hurt_volumes;
        }
        
        UIFrameData ui_frame_data = {0};
        {
            ui_frame_data.delta_t = app->delta_t;
            ui_frame_data.widget_arena = &app->frame_arena;
            ui_frame_data.main_font = &app->main_font;
            ui_frame_data.editor_font = &app->editor_font;
            ui_frame_data.input = &app->input;
        }
        UIBeginFrame(&app->ui, &ui_frame_data, app->render_w, app->render_h);
        {
            StateType next_state_type = StateUpdate(app->state_type, &app->state_arena);
            if(app->next_state_type == STATE_invalid)
            {
                app->next_state_type = next_state_type;
            }
            app->state_change_transition +=
                ((f32)(!!(app->next_state_type != STATE_invalid)) - app->state_change_transition) * app->delta_t;
        }
        UIEndFrame(&app->ui, &app->renderer);
        
        if(app->next_state_type != STATE_invalid)
        {
            if(app->state_change_transition > 0.95f)
            {
                StateCleanUp(app->state_type, &app->state_arena);
                MemoryArenaClear(&app->state_arena);
                app->state_type = app->next_state_type;
                app->next_state_type = STATE_invalid;
                StateInit(app->state_type, &app->state_arena);
            }
        }
        
        if(app->state_change_transition > 0.001f)
        {
            RendererPushFilledRect2D(&app->renderer, v4(0, 0, 0, 1.2f*app->state_change_transition),
                                     v4(0, 0, app->render_w, app->render_h));
        }
    }
    RendererEndFrame(&app->renderer);
    
    AudioUpdate(&app->audio);
    
    platform->SwapBuffers();
    
    if(platform->key_pressed[KEY_f11])
    {
        platform->fullscreen = !platform->fullscreen;
    }
    
    // NOTE(rjf): Update platform data
    {
        platform->last_mouse_x = platform->mouse_x;
        platform->last_mouse_y = platform->mouse_y;
        
        // NOTE(rjf): Prepare input data for frame
        {
            platform->last_key = 0;
            for(u32 i = 0; i < KEY_MAX; ++i)
            {
                platform->key_pressed[i] = 0;
            }
            platform->keyboard_used = 0;
            
            platform->mouse_scroll_x = 0.f;
            platform->mouse_scroll_y = 0.f;
            platform->left_mouse_pressed = 0;
            platform->right_mouse_pressed = 0;
        }
    }
}