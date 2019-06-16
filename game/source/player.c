
internal void
PlayerInit(Player *player, v3 spawn)
{
    player->state = PLAYER_STATE_free;
    player->sphere = SphereComponentInit(spawn, 1.f);
    player->sprite = SpriteComponentInit(spawn, 1.f, &app->art, v4(160, 240, 16, 16), 0, v4u(1));
    player->attack = AttackComponentInit(ATTACK_TYPE_melee, PlayerEntityID());
    player->weapon = WeaponComponentInit(&app->art, v4(0, 0, 0, 0));
    player->health = HealthComponentInit(spawn, 1.f, 1.f, PlayerEntityID());
    player->weapon_item = 0;
    player->attack_velocity_boost = 1.f;
    MemorySet(player->inventory, 0, sizeof(player->inventory));
}

internal b32
PlayerGetItem(Player *player, ItemType *item)
{
    b32 result = 0;
    for(u32 i = 0; i < ArrayCount(player->inventory); ++i)
    {
        if(player->inventory[i] == 0)
        {
            result = 1;
            player->inventory[i] = item;
            player->state = PLAYER_STATE_get_item;
            player->get_item_wait_time = 2.f;
            player->get_item_item = item;
            break;
        }
    }
    return result;
}

internal void
PlayerUpdate(Player *player, Map *map, Camera *camera, v3 last_safe_player_position,
             ProjectileSet *projectiles, ParticleMaster *particles)
{
    b32 controls_enabled = (player->state == PLAYER_STATE_free);
    
    if(controls_enabled)
    {
        f32 move_speed = 150.f;
        f32 horizontal_movement = 0;
        f32 vertical_movement = 0;
        
        if(platform->key_down[KEY_w])
        {
            vertical_movement -= 1;
        }
        if(platform->key_down[KEY_s])
        {
            vertical_movement += 1;
        }
        if(platform->key_down[KEY_a])
        {
            horizontal_movement -= 1;
        }
        if(platform->key_down[KEY_d])
        {
            horizontal_movement += 1;
        }
        if(platform->key_pressed[KEY_space])
        {
            player->sphere.velocity.y = 15;
        }
        
        if(UIIDEqual(app->ui.hot, UIIDNull()) && 
           UIIDEqual(app->ui.active, UIIDNull()) &&
           platform->left_mouse_pressed)
        {
            f32 attack_angle = 0.f;
            
            // NOTE(rjf): Calculate attack angle
            {
                v4 mouse_ray_clip_1 = {
                    2*platform->mouse_x/app->render_w - 1,
                    -(2*platform->mouse_y/app->render_h - 1),
                    0.f,
                    1.f,
                };
                
                v4 mouse_ray_clip_2 = {
                    2*platform->mouse_x/app->render_w - 1,
                    -(2*platform->mouse_y/app->render_h - 1),
                    0.1f,
                    1.f,
                };
                
                m4 view = M4LookAt(camera->eye, camera->look_at_position, v3(0, 1, 0));
                m4 projection = M4Perspective(90.f, app->render_w / app->render_h, 1.f, 1000.f);
                m4 view_projection = M4MultiplyM4(projection, view);
                m4 inverse_view_projection = M4Inverse(view_projection);
                
                v4 mouse_ray_world_1 = V4MultiplyM4(mouse_ray_clip_1, inverse_view_projection);
                mouse_ray_world_1.x /= mouse_ray_world_1.w;
                mouse_ray_world_1.y /= mouse_ray_world_1.w;
                mouse_ray_world_1.z /= mouse_ray_world_1.w;
                v4 mouse_ray_world_2 = V4MultiplyM4(mouse_ray_clip_2, inverse_view_projection);
                mouse_ray_world_2.x /= mouse_ray_world_2.w;
                mouse_ray_world_2.y /= mouse_ray_world_2.w;
                mouse_ray_world_2.z /= mouse_ray_world_2.w;
                
                v3 mouse_ray = mouse_ray_world_1.xyz;
                v3 mouse_ray_change = V3MinusV3(mouse_ray_world_2.xyz, mouse_ray);
                
                for(u32 i = 0; i < 100; ++i)
                {
                    mouse_ray.x += mouse_ray_change.x;
                    mouse_ray.y += mouse_ray_change.y;
                    mouse_ray.z += mouse_ray_change.z;
                    
                    if(mouse_ray.y < player->sphere.position.y)
                    {
                        break;
                    }
                }
                
                v2 world_spot = { mouse_ray.x, mouse_ray.z };
                
                attack_angle = -ArcTan2F(world_spot.y - player->sphere.position.z,
                                         world_spot.x - player->sphere.position.x) + PIf/2;
            }
            
            if(WeaponComponentAttack(&player->weapon, attack_angle))
            {
                player->sphere.velocity.x += player->attack_velocity_boost*20.f*CosF(-attack_angle + PIf/2);
                player->sphere.velocity.z += player->attack_velocity_boost*20.f*SinF(-attack_angle + PIf/2);
                player->attack_velocity_boost -= 0.2f;
            }
            
            AttackComponentAttack(&player->attack, v3(1.f*CosF(PIf/2-attack_angle), 0, 1.f*SinF(PIf/2-attack_angle)),
                                  2.f, 0.2f);
        }
        
        player->attack_velocity_boost += (1.f - player->attack_velocity_boost) * app->delta_t;
        
        f32 movement_length = SquareRoot(horizontal_movement*horizontal_movement + vertical_movement*vertical_movement);
        
        if(movement_length)
        {
            horizontal_movement /= movement_length;
            vertical_movement /= movement_length;
        }
        
        player->sphere.velocity.x += horizontal_movement*move_speed*app->delta_t;
        player->sphere.velocity.z += vertical_movement  *move_speed*app->delta_t;
        
    }
    
    UpdateSphereComponents(&player->sphere, map, 1);
    TrackSpriteComponentsToSphereComponents(&player->sprite, &player->sphere, 1, map);
    TrackWeaponComponentsToSphereComponents(&player->weapon, &player->sphere, 1);
    UpdateSpriteComponents(&player->sprite, 1);
    UpdateAttackComponents(&player->attack, 1, projectiles);
    TrackAttackComponentsToSphereComponents(&player->attack, &player->sphere, 1);
    
    player->weapon.bob_magnitude = player->sprite.bob_magnitude;
    
    if(player->sphere.velocity.y < -25.f)
    {
        if(player->sphere.velocity.y < -50.f)
        {
            player->sphere.position = last_safe_player_position;
            player->sphere.velocity = v3(0, 0, 0);
        }
    }
    else
    {
        camera->look_at_position_target = player->sphere.position;
        camera->look_at_position_target.y -= 1.f;
    }
    
    if(player->state == PLAYER_STATE_get_item)
    {
        player->get_item_wait_time -= app->delta_t;
        if(player->get_item_wait_time <= 0.f)
        {
            player->state = PLAYER_STATE_free;
            
            if(player->weapon_item == 0 && player->get_item_item->flags & ITEM_FLAG_weapon)
            {
                player->weapon_item = player->get_item_item;
                player->weapon = WeaponComponentInitFromItemType(player->get_item_item);
            }
            
        }
    }
    
    TrackHealthComponentsToSphereComponents(&player->health, &player->sphere, 1);
    UpdateHealthComponents(&player->health, 1);
    TrackSpriteComponentsToHealthComponents(&player->sprite, &player->health, 1);
    CollideHealthAndSphereComponentsWithProjectiles(&player->health, &player->sphere, 1, projectiles,
                                                    particles);
}

internal void
PlayerRender(Player *player, ParticleMaster *particles)
{
    RenderSpriteComponents(&player->sprite, 1);
    RenderWeaponComponents(&player->weapon, 1);
    
    if(player->state == PLAYER_STATE_get_item)
    {
        ItemType *item = player->get_item_item;
        v3 center = player->sphere.position;
        center.y += 4.f;
        center.y += ClampF32(-1.5f*player->get_item_wait_time, -100.f, -2.f);
        v2 scale = { item->source.width / 16.f, item->source.height / 16.f };
        v3 p1 = { center.x - scale.x, center.y - scale.y, center.z };
        v3 p2 = { center.x - scale.x, center.y + scale.y, center.z - 1.f };
        v3 p3 = { center.x + scale.x, center.y - scale.y, center.z };
        v3 p4 = { center.x + scale.x, center.y + scale.y, center.z - 1.f };
        RendererPushTexture(&app->renderer, &app->art, 0, item->source,
                            p1, p2, p3, p4, v4u(1));
        
        center.z -= 0.8f;
        center.y -= 0.4f;
        RendererPushPointLight(&app->renderer, v3(center.x, center.y + 1.f, center.z + 1.f),
                               v3(1.f, 0.8f, 0.6f), 4.f, 1.f);
        ParticleSpawn(particles, PARTICLE_TYPE_strike, center, v3(RandomF32(-1, 1), RandomF32(-1, 1), RandomF32(-1, 1)));
    }
    
    RenderSphereComponentsDebug(&player->sphere, 1);
    RenderAttackComponentsDebug(&player->attack, 1);
}