typedef struct GameState GameState;
struct GameState
{
    b32 paused;
    b32 settings_menu_open;
    SettingsMenu settings_menu;
    
    Player player;
    Camera camera;
    EntitySets entities;
    ParticleMaster particles;
    ProjectileSet projectiles;
    Map map;
    
    f32 lower_wall_peek_transition_start;
    f32 lower_wall_peek_transition_end;
    f32 lower_wall_peek_transition_time;
    f32 lower_wall_peek_transition;
    b32 lower_wall_present;
    
    v3 last_safe_player_position;
    
    AudioSourceID background_music_source;
};

internal void
GameStateInit(GameState *state)
{
    state->paused = 0;
    state->settings_menu_open = 0;
    
    LoadedMap *map = &global_intro_map;
    EntitySetsInit(&state->entities, &app->state_arena);
    state->particles = ParticleMasterInit(&app->state_arena);
    ProjectileSetInit(&state->projectiles, &app->state_arena, MAX_PROJECTILES);
    state->map = MapInitFromLoadedMap(&app->state_arena, &state->entities, map);
    
    PlayerInit(&state->player, v3(map->player_spawn.x*TILE_SIZE + 1.f,
                                  MapSampleHeight(&state->map, v2(map->player_spawn.x*TILE_SIZE + 1.f, map->player_spawn.y*TILE_SIZE + 1.f)) + 1.5f,
                                  map->player_spawn.y*TILE_SIZE + 1.f));
    CameraInit(&state->camera, v3(map->player_spawn.x*TILE_SIZE + 1.f, 15,
                                  map->player_spawn.y*TILE_SIZE + 1.f));
    
    state->last_safe_player_position = state->player.sphere.position;
    state->lower_wall_peek_transition_start = 0.f;
    state->lower_wall_peek_transition_end = 0.f;
    state->lower_wall_peek_transition_time = 0.f;
    state->lower_wall_peek_transition = 0.f;
    
    state->background_music_source = AudioReserveSource(&app->audio);
    AudioPlaySource(&app->audio, state->background_music_source, &app->music_1,
                    AUDIO_background_music, 0.6f, 1.f, 1);
}

internal void
GameStateCleanUp(GameState *state)
{
    MapCleanUp(&state->map);
    AudioUnreserveSource(&app->audio, state->background_music_source);
}

internal StateType
GameStateUpdate(GameState *state)
{
    StateType next_state = STATE_invalid;
    
    app->audio.listener_position = state->player.sphere.position;
    
    if(platform->key_pressed[KEY_esc])
    {
        state->paused = !state->paused;
    }
    
    if(state->paused)
    {
        if(state->settings_menu_open)
        {
            state->settings_menu_open = SettingsMenuUpdate(&state->settings_menu);
        }
        else
        {
            UIPushCenteredColumn(&app->ui, v2(400, 50), 5);
            {
                UITitle(&app->ui, "PAUSED");
                UIDivider(&app->ui);
                
                if(UIButton(&app->ui, "Resume"))
                {
                    state->paused = 0;
                }
                if(UIButton(&app->ui, "Settings"))
                {
                    state->settings_menu_open = 1;
                    state->settings_menu.state = SETTINGS_MENU_main;
                }
                if(UIButton(&app->ui, "Quit"))
                {
                    next_state = STATE_title;
                }
            }
            UIPopColumn(&app->ui);
        }
    }
    else
    {
        
        PlayerUpdate(&state->player, &state->map, &state->camera, state->last_safe_player_position,
                     &state->projectiles, &state->particles);
        CameraUpdate(&state->camera);
        EntitySetsDoDefaultUpdate(&state->entities, &state->map, &state->player, &state->projectiles,
                                  &state->particles);
        ParticleMasterUpdate(&state->particles);
        ProjectileSetUpdate(&state->projectiles, &state->map, &state->particles);
        
        RendererPushPointLight(&app->renderer,
                               v3(state->player.sphere.position.x,
                                  state->player.sphere.position.y + 1.f,
                                  state->player.sphere.position.z + 0.1f),
                               v3(1.f, 1.f, 1.f), 25.f, 1.5f);
        
        // NOTE(rjf): Update lower-wall peek
        {
            Player *player = &state->player;
            
            i32 tile_x = (i32)(player->sphere.position.x / TILE_SIZE);
            i32 tile_y = (i32)(player->sphere.position.z / TILE_SIZE);
            
            if(MapGetTile(&state->map, iv2(tile_x, tile_y+2)).flags & MAP_TILE_WALL ||
               MapGetTile(&state->map, iv2(tile_x, tile_y+1)).flags & MAP_TILE_WALL)
            {
                if(!state->lower_wall_present)
                {
                    state->lower_wall_peek_transition_start = state->lower_wall_peek_transition;
                    state->lower_wall_peek_transition_end = 1.f;
                    state->lower_wall_peek_transition_time = 0.f;
                }
                state->lower_wall_present = 1;
            }
            else
            {
                if(state->lower_wall_present)
                {
                    state->lower_wall_peek_transition_start = state->lower_wall_peek_transition;
                    state->lower_wall_peek_transition_end = 0.f;
                    state->lower_wall_peek_transition_time = 0.f;
                }
                state->lower_wall_present = 0;
            }
            
            state->lower_wall_peek_transition_time += app->delta_t;
            state->lower_wall_peek_transition =
                state->lower_wall_peek_transition_start + (state->lower_wall_peek_transition_end - state->lower_wall_peek_transition_start)*
                InterpolateSmooth(state->lower_wall_peek_transition_time);
            
            state->camera.eye.z -= state->lower_wall_peek_transition*8.f;
            state->camera.eye.y += state->lower_wall_peek_transition*4.f;
        }
        
        // NOTE(rjf): Entity-specific updates
        {
            Player *player = &state->player;
            
            // NOTE(rjf): Torches
            {
                TorchSet *torches = &state->entities.torch_set;
                for(u32 i = 0; i < torches->count; ++i)
                {
                    RendererPushPointLight(&app->renderer,
                                           V3AddV3(torches->sphere[i].position, v3(0, 1.f, 0.1f)),
                                           v3(1, 0.98f, 0.8f), 5.f, 1.f);
                    for(u32 j = 0; j < 1; ++j)
                    {
                        ParticleSpawn(&state->particles, PARTICLE_TYPE_fire,
                                      v3(torches->sphere[i].position.x + RandomF32(-0.2f, 0.2f),
                                         torches->sphere[i].position.y + 0.8f,
                                         torches->sphere[i].position.z + RandomF32(-0.2f, 0.2f) - 0.8f),
                                      v3(RandomF32(-0.2f, 0.2f), RandomF32(0.5f, 1), RandomF32(-0.2f, 0.2f)));
                    }
                }
            }
            
            // NOTE(rjf): Collectibles
            {
                CollectibleSet *collectibles = &state->entities.collectible_set;
                for(u32 i = 0; i < collectibles->count; ++i)
                {
                    v3 player_to_collectible = {
                        collectibles->static_float[i].position.x - player->sphere.position.x,
                        collectibles->static_float[i].position.y - player->sphere.position.y,
                        collectibles->static_float[i].position.z - player->sphere.position.z,
                    };
                    
                    f32 player_to_collectible_distance_squared =
                        V3Dot(player_to_collectible, player_to_collectible);
                    f32 sum_radii_squared = 2;
                    
                    if(player_to_collectible_distance_squared < sum_radii_squared)
                    {
                        PlayerGetItem(&state->player, collectibles->item[i]);
                        CollectibleSetRemoveByIndex(collectibles, i);
                    }
                }
            }
            
            // NOTE(rjf): Treasure Chests
            {
                TreasureChestSet *treasure_chests = &state->entities.treasure_chest_set;
                for(u32 i = 0; i < treasure_chests->count; ++i)
                {
                    if(treasure_chests->item[i])
                    {
                        v3 player_to_treasure_chest = {
                            treasure_chests->sphere[i].position.x - player->sphere.position.x,
                            treasure_chests->sphere[i].position.y - player->sphere.position.y,
                            treasure_chests->sphere[i].position.z - player->sphere.position.z,
                        };
                        
                        f32 player_to_treasure_chest_distance_squared =
                            V3Dot(player_to_treasure_chest, player_to_treasure_chest);
                        f32 sum_radii_squared = 2;
                        
                        if(player_to_treasure_chest_distance_squared < sum_radii_squared)
                        {
                            AudioPlaySoundAtPoint(&app->audio, &app->open_1, AUDIO_master,
                                                  RandomF32(0.8f, 1.2f),
                                                  RandomF32(0.8f, 1.2f),
                                                  state->player.sphere.position);
                            PlayerGetItem(&state->player, treasure_chests->item[i]);
                            treasure_chests->sprite[i].source.y += 16;
                            treasure_chests->item[i] = 0;
                        }
                    }
                }
            }
        }
        
        // NOTE(rjf): Check player health for game overs.
        {
            Player *player = &state->player;
            if(player->health.health <= 0.f)
            {
                player->state = PLAYER_STATE_dead;
                next_state = STATE_game_over;
            }
        }
    }
    
    // NOTE(rjf): 3D rendering
    {
        v3 shadow_target = {
            state->camera.look_at_position.x + 32.f - FModF(state->camera.look_at_position.x, 32.f),
            state->camera.look_at_position.y + 32.f - FModF(state->camera.look_at_position.y, 32.f),
            state->camera.look_at_position.z + 32.f - FModF(state->camera.look_at_position.z, 32.f),
        };
        
        v3 shadow_eye = {
            shadow_target.x+10,
            shadow_target.y+40,
            shadow_target.z+10,
        };
        
        m4 view = M4LookAt(state->camera.eye, state->camera.look_at_position, v3(0, 1, 0));
        m4 projection = M4Perspective(90.f, app->render_w / app->render_h, 1.f, 1000.f);
        m4 shadow_view = M4LookAt(shadow_eye, shadow_target, v3(0, 1, 0));
        m4 shadow_projection = M4Orthographic(-64.f, 64.f, -32.f, 96.f, 1.f, 200.f);
        RendererPushViewMatrix(&app->renderer, view);
        RendererPushProjectionMatrix(&app->renderer, projection);
        RendererPushShadowViewMatrix(&app->renderer, shadow_view);
        RendererPushShadowProjectionMatrix(&app->renderer, shadow_projection);
        {
            MapRender(&state->map);
            PlayerRender(&state->player, &state->particles);
            EntitySetsRender(&state->entities);
            ParticleMasterRender(&state->particles);
            ProjectileSetRender(&state->projectiles, &state->particles);
        }
    }
    
    if(state->paused)
    {
        RendererPushFilledRect2D(&app->renderer, v4(0, 0, 0, 0.1f), v4(0, 0, app->render_w, app->render_h));
    }
    
    return next_state;
}