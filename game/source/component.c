internal void
UpdateSphereComponents(SphereComponent *sphere, Map *map, u32 n)
{
    f32 resolve_rate = platform->target_frames_per_second;
    
    for(u32 i = 0; i < n; ++i, ++sphere)
    {
        f32 height = MapSampleHeight(map, v2(sphere->position.x, sphere->position.z));
        v3 collision_resolve_velocity = {0};
        v3 velocity_sum = V3AddV3(sphere->velocity, sphere->hit_velocity);
        
        for(f32 f = 0.1f; f <= 1.f; f += 0.1f)
        {
            
            v2 test_position = {
                sphere->position.x + velocity_sum.x*app->delta_t*f,
                sphere->position.z + velocity_sum.z*app->delta_t*f,
            };
            
            iv2 tile_search_lower_bound = {
                (i32)((test_position.x - sphere->radius) / TILE_SIZE),
                (i32)((test_position.y - sphere->radius) / TILE_SIZE),
            };
            
            iv2 tile_search_upper_bound = {
                (i32)((test_position.x + sphere->radius) / TILE_SIZE),
                (i32)((test_position.y + sphere->radius) / TILE_SIZE),
            };
            
            b32 collision = 0;
            
            for(i32 j = tile_search_lower_bound.y; j <= tile_search_upper_bound.y; ++j)
            {
                for(i32 i = tile_search_lower_bound.x; i <= tile_search_upper_bound.x; ++i)
                {
                    if(MapGetTile(map, iv2(i, j)).flags & MAP_TILE_WALL ||
                       (app->frame_index > 0 &&
                        !(MapGetTile(map, iv2(i, j)).flags & MAP_TILE_PIT) &&
                        sphere->position.y < height+1.f))
                    {
                        v4 rect = {
                            i*TILE_SIZE, j*TILE_SIZE,
                            TILE_SIZE, TILE_SIZE,
                        };
                        
                        f32 test_radius = sphere->radius;
                        
                        v2 closest_point_in_rect = {
                            ClampF32(test_position.x, rect.x, rect.x + rect.width),
                            ClampF32(test_position.y, rect.y, rect.y + rect.height),
                        };
                        
                        v2 closest_point_to_circle = {
                            test_position.x - closest_point_in_rect.x,
                            test_position.y - closest_point_in_rect.y,
                        };
                        
                        f32 test_radius_squared = test_radius*test_radius;
                        f32 distance_squared = V2LengthSquared(closest_point_to_circle);
                        
                        if(distance_squared < test_radius_squared)
                        {
                            v2 circle_to_closest_point = {
                                closest_point_in_rect.x - test_position.x,
                                closest_point_in_rect.y - test_position.y,
                            };
                            
                            v2 circle_to_closest_point_normalized = V2Normalize(circle_to_closest_point);
                            
                            v2 overlap = {
                                test_radius*circle_to_closest_point_normalized.x - circle_to_closest_point.x,
                                test_radius*circle_to_closest_point_normalized.y - circle_to_closest_point.y,
                            };
                            
                            if(overlap.x > 0 && MapGetTile(map, iv2(i-1, j)).flags & MAP_TILE_WALL)
                            {
                                overlap.x = 0;
                            }
                            else if(overlap.x < 0 && MapGetTile(map, iv2(i+1, j)).flags & MAP_TILE_WALL)
                            {
                                overlap.x = 0;
                            }
                            
                            if(overlap.y > 0 && MapGetTile(map, iv2(i, j-1)).flags & MAP_TILE_WALL)
                            {
                                overlap.y = 0;
                            }
                            else if(overlap.y < 0 && MapGetTile(map, iv2(i, j+1)).flags & MAP_TILE_WALL)
                            {
                                overlap.y = 0;
                            }
                            
                            collision_resolve_velocity.x -= overlap.x*resolve_rate;
                            collision_resolve_velocity.z -= overlap.y*resolve_rate;
                            collision = 1;
                        }
                    }
                }
            }
            
            if(collision)
            {
                break;
            }
        }
        
        sphere->position.x += velocity_sum.x * app->delta_t;
        sphere->position.y += velocity_sum.y * app->delta_t;
        sphere->position.z += velocity_sum.z * app->delta_t;
        
        sphere->position.x += collision_resolve_velocity.x * app->delta_t;
        sphere->position.y += collision_resolve_velocity.y * app->delta_t;
        sphere->position.z += collision_resolve_velocity.z * app->delta_t;
        
        sphere->velocity.x -= sphere->velocity.x * app->delta_t * 16.f;
        sphere->velocity.y -= 50.f * app->delta_t;
        sphere->velocity.z -= sphere->velocity.z * app->delta_t * 16.f;
        
        sphere->hit_velocity.x -= sphere->hit_velocity.x * app->delta_t * 10.f;
        sphere->hit_velocity.y -= sphere->hit_velocity.y * app->delta_t * 10.f;
        sphere->hit_velocity.z -= sphere->hit_velocity.z * app->delta_t * 10.f;
        
        b32 over_ground = 0;
        {
            iv2 lower_tile_search_bound = {
                (i32)((sphere->position.x - sphere->radius + 0.4f) / TILE_SIZE),
                (i32)((sphere->position.z - sphere->radius + 0.4f) / TILE_SIZE),
            };
            
            iv2 upper_tile_search_bound = {
                (i32)((sphere->position.x + sphere->radius - 0.4f) / TILE_SIZE),
                (i32)((sphere->position.z + sphere->radius - 0.4f) / TILE_SIZE),
            };
            
            for(i32 i = lower_tile_search_bound.x; i <= upper_tile_search_bound.x; ++i)
            {
                for(i32 j = lower_tile_search_bound.y; j <= upper_tile_search_bound.y; ++j)
                {
                    if(!(MapGetTile(map, iv2(i, j)).flags & MAP_TILE_PIT))
                    {
                        over_ground = 1;
                        break;
                    }
                }
            }
        }
        
        if(over_ground && sphere->position.y < height + 1.25f)
        {
            sphere->position.y = height + 1.25f;
            sphere->velocity.y = 0;
        }
        
        
    }
}

internal void
TrackSpriteComponentsToSphereComponents(SpriteComponent *sprite, SphereComponent *sphere, u32 n, Map *map)
{
    for(u32 i = 0; i < n; ++i, ++sprite, ++sphere)
    {
        sprite->position = sphere->position;
        
        f32 distance_from_ground = sphere->position.y -
            MapSampleHeight(map, v2(sphere->position.x, sphere->position.z));
        f32 velocity_magnitude = SquareRoot(sphere->velocity.x*sphere->velocity.x +
                                            sphere->velocity.z*sphere->velocity.z);
        
        if(distance_from_ground > 2.f)
        {
            sprite->bob_magnitude = 0.f;
        }
        else
        {
            if(AbsoluteValue(velocity_magnitude) > 1.f)
            {
                sprite->bob_magnitude += (100.f - sprite->bob_magnitude) * app->delta_t * 28.f;
            }
            else
            {
                sprite->bob_magnitude += (0.f - sprite->bob_magnitude) * app->delta_t * 28.f;
            }
        }
        
    }
}

internal void
TrackSpriteComponentsToStaticFloatComponents(SpriteComponent *sprite, StaticFloatComponent *static_float, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++sprite, ++static_float)
    {
        sprite->position = static_float->position;
        sprite->position.y += 1.2f + 0.5f*SinF(static_float->float_sin_pos);
    }
}

internal void
TrackWeaponComponentsToSphereComponents(WeaponComponent *weapon, SphereComponent *sphere, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++weapon, ++sphere)
    {
        weapon->position = sphere->position;
    }
}

internal void
UpdateSpriteComponents(SpriteComponent *sprite, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++sprite)
    {
        sprite->bob_sin_pos += sprite->bob_magnitude * 0.4f * app->delta_t;
        sprite->position.y += sprite->bob_magnitude * 0.001f * SinF(sprite->bob_sin_pos);
    }
}

internal void
RenderSpriteComponents(SpriteComponent *sprite, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++sprite)
    {
        v3 center = sprite->position;
        v2 scale = { sprite->source.width / 16.f, sprite->source.height / 16.f };
        v3 p1 = { center.x - scale.x, center.y - scale.y + scale.y - 1.f, center.z };
        v3 p2 = { center.x - scale.x, center.y + scale.y + scale.y - 1.f, center.z - 1.f };
        v3 p3 = { center.x + scale.x, center.y - scale.y + scale.y - 1.f, center.z };
        v3 p4 = { center.x + scale.x, center.y + scale.y + scale.y - 1.f, center.z - 1.f };
        RendererPushTexture(&app->renderer, sprite->texture, sprite->flags, sprite->source,
                            p1, p2, p3, p4, sprite->tint);
    }
}

internal void
AttackComponentAttack(AttackComponent *attack, v3 position, f32 max_radius, f32 attack_time)
{
    attack->hit_sphere_relative_position = position;
    attack->hit_sphere_max_radius = max_radius;
    attack->hit_sphere_radius = 0.f;
    attack->attack_time = attack_time;
    attack->active = 1;
    
    AudioPlaySoundAtPoint(&app->audio, &app->swing_1, AUDIO_master,
                          RandomF32(0.2f, 0.4f),
                          RandomF32(0.8f, 1.2f),
                          attack->position);
}

internal void
UpdateAttackComponents(AttackComponent *attack, u32 n, ProjectileSet *projectiles)
{
    for(u32 i = 0; i < n; ++i, ++attack)
    {
        switch(attack->type)
        {
            
            case ATTACK_TYPE_melee:
            {
                if(attack->attack_time > 0.f)
                {
                    attack->attack_time -= app->delta_t;
                    attack->hit_sphere_radius = ClampF32(
                        attack->hit_sphere_max_radius*(-(2*attack->attack_time-1)*(2*attack->attack_time-1)+1),
                        0.f, attack->hit_sphere_max_radius);
                    attack->active = 1;
                }
                else
                {
                    attack->active = 0;
                }
                break;
            }
            
            case ATTACK_TYPE_projectile_fire:
            {
                if(attack->attack_time > 0.f)
                {
                    attack->attack_time -= app->delta_t;
                }
                else
                {
                    attack->attack_time = RandomF32(2.f, 6.f);
                    
                    v3 attack_to_target = {
                        attack->target_position.x - attack->position.x,
                        attack->target_position.y - attack->position.y,
                        attack->target_position.z - attack->position.z,
                    };
                    
                    v3 attack_to_target_normalized = V3Normalize(attack_to_target);
                    
                    v3 projectile_pos = {
                        attack->position.x + 2.f*attack_to_target_normalized.x,
                        attack->position.y + 2.f*attack_to_target_normalized.y,
                        attack->position.z + 2.f*attack_to_target_normalized.z,
                    };
                    
                    f32 speed = 12.f;
                    
                    v3 projectile_velocity = {
                        speed * attack_to_target_normalized.x,
                        speed * attack_to_target_normalized.y,
                        speed * attack_to_target_normalized.z,
                    };
                    
                    ProjectileSpawn(projectiles, PROJECTILE_TYPE_fire, projectile_pos,
                                    projectile_velocity, DAMAGE_fire, 1.f);
                }
                break;
            }
            
            default: break;
        }
    }
}

internal void
TrackAttackComponentsToSphereComponents(AttackComponent *attack, SphereComponent *sphere, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++attack, ++sphere)
    {
        attack->hit_sphere_position = attack->position = sphere->position;
        attack->hit_sphere_position.x += attack->hit_sphere_relative_position.x;
        attack->hit_sphere_position.y += attack->hit_sphere_relative_position.y;
        attack->hit_sphere_position.z += attack->hit_sphere_relative_position.z;
    }
}

internal void
RenderWeaponComponents(WeaponComponent *weapon, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++weapon)
    {
        v3 center = weapon->position;
        weapon->bob_sin_pos += weapon->bob_magnitude * 0.4f * app->delta_t;
        f32 bob_sin = SinF(weapon->bob_sin_pos);
        v2 scale = { weapon->source.width / 16.f, weapon->source.height / 16.f };
        
        weapon->attack_transition -= 7.f * app->delta_t;
        
        v4 p1 = { -scale.x, -scale.y, 0.f, 1 };
        v4 p2 = { -scale.x, +scale.y, 0.f, 1 };
        v4 p3 = { +scale.x, -scale.y, 0.f, 1 };
        v4 p4 = { +scale.x, +scale.y, 0.f, 1 };
        
        m4 weapon_transform = M4InitD(1.f);
        
        if(weapon->attack_transition < 0.01f)
        {
            center.x += 0.5f;
            center.y -= 0.4f;
            center.z += 0.2f;
            center.y += weapon->bob_magnitude * 0.001f * bob_sin;
            
            weapon_transform = M4MultiplyM4(weapon_transform, M4TranslateV3(center));
            weapon_transform = M4MultiplyM4(weapon_transform, M4Rotate(-PIf/2 + bob_sin/10.f, v3(0, 0, 1)));
            weapon_transform = M4MultiplyM4(weapon_transform, M4Rotate(PIf/10, v3(1, 0, 0)));
        }
        else
        {
            p1 = v4( -scale.x, 0.f, -scale.y, 1 );
            p2 = v4( -scale.x, 0.f, +scale.y, 1 );
            p3 = v4( +scale.x, 0.f, -scale.y, 1 );
            p4 = v4( +scale.x, 0.f, +scale.y, 1 );
            
            center.x += (-1.2f + 1.f);
            center.y -= 0.4f;
            center.z += (-0.9f + 1.f);
            center.y += weapon->bob_magnitude * 0.001f * bob_sin;
            
            weapon_transform = M4MultiplyM4(weapon_transform, M4TranslateV3(center));
            weapon_transform = M4MultiplyM4(weapon_transform, M4Rotate(-PIf/3 + PIf*(1-weapon->attack_transition)*0.6f, v3(0, 1, 0)));
            weapon_transform = M4MultiplyM4(weapon_transform, M4Rotate(weapon->attack_angle, v3(0, 1, 0)));
            weapon_transform = M4MultiplyM4(weapon_transform, M4TranslateV3(v3(-0.2f, 0.f, 1.2f)));
        }
        
        p1 = V4MultiplyM4(p1, weapon_transform);
        p2 = V4MultiplyM4(p2, weapon_transform);
        p3 = V4MultiplyM4(p3, weapon_transform);
        p4 = V4MultiplyM4(p4, weapon_transform);
        
        RendererPushTexture(&app->renderer, weapon->texture, 0, weapon->source,
                            p1.xyz, p2.xyz, p3.xyz, p4.xyz, v4u(1));
    }
}

internal b32
WeaponComponentAttack(WeaponComponent *weapon, f32 angle)
{
    b32 result = 0;
    {
        weapon->attack_transition = 1.f;
        weapon->attack_angle = angle;
        result = 1;
    }
    return result;
}

internal void
UpdateStaticFloatComponents(StaticFloatComponent *static_float, Map *map, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++static_float)
    {
        static_float->float_sin_pos += app->delta_t;
        f32 height = MapSampleHeight(map, v2(static_float->position.x, static_float->position.z));
        if(static_float->position.y < height + 1.25f)
        {
            static_float->position.y = height + 1.25f;
        }
    }
}

internal WeaponComponent
WeaponComponentInitFromItemType(ItemType *item)
{
    WeaponComponent weapon = {0};
    if(item->flags & ITEM_FLAG_weapon)
    {
        weapon.texture = &app->art;
        weapon.source = item->source;
    }
    return weapon;
}

internal void
TrackEnemyAIComponentsToSphereComponents(EnemyAIComponent *enemy_ai, SphereComponent *sphere, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++enemy_ai, ++sphere)
    {
        enemy_ai->position = sphere->position;
    }
}

internal void
UpdateEnemyAI(EnemyAIComponent *enemy_ai, AttackComponent *attack, u32 n, Player *player)
{
    for(u32 i = 0; i < n; ++i, ++enemy_ai, ++attack)
    {
        v3 enemy_to_player = {
            player->sphere.position.x - enemy_ai->position.x,
            player->sphere.position.y - enemy_ai->position.y,
            player->sphere.position.z - enemy_ai->position.z,
        };
        
        f32 enemy_to_player_distance_squared = V3Dot(enemy_to_player, enemy_to_player);
        
        attack->target_position = player->sphere.position;
        
        switch(enemy_ai->state)
        {
            case ENEMY_AI_STATE_calm:
            {
                
                if(enemy_to_player_distance_squared <= 10.f*10.f)
                {
                    enemy_ai->state = ENEMY_AI_STATE_attacking;
                    enemy_ai->player_close = 0;
                }
                else
                {
                    
                    if(enemy_ai->time_to_movement > 0)
                    {
                        enemy_ai->time_to_movement -= app->delta_t;
                        if(enemy_ai->time_to_movement <= 0)
                        {
                            enemy_ai->movement_time = RandomF32(0.5f, 2.f);
                            f32 movement_angle = RandomF32(0.f, 2.f*PIf);
                            enemy_ai->movement.x = CosF(movement_angle);
                            enemy_ai->movement.z = SinF(movement_angle);
                        }
                    }
                    else
                    {
                        if(enemy_ai->movement_time > 0)
                        {
                            enemy_ai->movement_time -= app->delta_t;
                        }
                        else
                        {
                            enemy_ai->time_to_movement = RandomF32(2.f, 8.f);
                        }
                    }
                    
                }
                
                break;
            }
            
            case ENEMY_AI_STATE_attacking:
            {
                
                if(enemy_to_player_distance_squared >= 15.f*15.f)
                {
                    enemy_ai->state = ENEMY_AI_STATE_calm;
                }
                else
                {
                    enemy_ai->movement.y = 0;
                    
                    if(enemy_to_player_distance_squared <= 4.f*4.f)
                    {
                        if(!enemy_ai->player_close)
                        {
                            enemy_ai->player_close = 1;
                            if(RandomF32(0, 1) > 0.4f)
                            {
                                enemy_ai->movement.y = 20;
                            }
                        }
                    }
                    else
                    {
                        enemy_ai->player_close = 0;
                    }
                    
                    if(enemy_ai->type == ENEMY_AI_TYPE_melee)
                    {
                        
                        v3 enemy_to_player_normalized = V3Normalize(enemy_to_player);
                        enemy_ai->movement.x = 2.f*enemy_to_player_normalized.x;
                        enemy_ai->movement.z = 2.f*enemy_to_player_normalized.z;
                        enemy_ai->movement_time = 1.f;
                        
                        attack->active = 1;
                        attack->hit_sphere_position = enemy_ai->position;
                        attack->hit_sphere_relative_position = v3(0.5f*enemy_to_player_normalized.x, 0, 0.5f*enemy_to_player_normalized.z);
                        attack->hit_sphere_max_radius = 0.5f;
                        attack->hit_sphere_radius = 0.5f;
                        attack->attack_time = 0.5f;
                        
                    }
                    else if(enemy_ai->type == ENEMY_AI_TYPE_ranged)
                    {
                        if(enemy_to_player_distance_squared <= 7.f*7.f)
                        {
                            
                            v3 enemy_to_player_normalized = V3Normalize(enemy_to_player);
                            enemy_ai->movement.x = -1.5f*enemy_to_player_normalized.x;
                            enemy_ai->movement.z = -1.5f*enemy_to_player_normalized.z;
                            enemy_ai->movement_time = 1.f;
                            
                            attack->active = 1;
                            attack->hit_sphere_position = enemy_ai->position;
                            attack->hit_sphere_relative_position = v3(0.5f*enemy_to_player_normalized.x, 0, 0.5f*enemy_to_player_normalized.z);
                            attack->hit_sphere_max_radius = 0.5f;
                            attack->hit_sphere_radius = 0.5f;
                            attack->attack_time = 0.5f;
                            
                        }
                        else
                        {
                            enemy_ai->movement_time = 0.f;
                        }
                        
                    }
                }
                
                break;
            }
            
            default: break;
        }
    }
}

internal void
TrackSphereComponentsToEnemyAIComponents(SphereComponent *sphere, EnemyAIComponent *enemy_ai, u32 n)
{
    for(u32 i = 0; i < n; ++i,++sphere, ++enemy_ai)
    {
        sphere->velocity.y += enemy_ai->movement.y;
        
        f32 horizontal_movement = 0;
        f32 vertical_movement = 0;
        
        if(enemy_ai->movement_time > 0)
        {
            f32 move_speed = 50.f;
            horizontal_movement = enemy_ai->movement.x;
            vertical_movement = enemy_ai->movement.z;
            sphere->velocity.x += horizontal_movement*move_speed*app->delta_t;
            sphere->velocity.z += vertical_movement  *move_speed*app->delta_t;
        }
    }
}

internal void
CollideHealthAndSphereComponentsWithAttackComponents(HealthComponent *health, SphereComponent *sphere,
                                                     u32 count, AttackComponent *attack, u32 attack_count,
                                                     ParticleMaster *particles)
{
    AttackComponent *attack_backup = attack;
    
    for(u32 i = 0; i < count; ++i, ++health, ++sphere)
    {
        if(health->hurt_transition < 0.1f)
        {
            attack = attack_backup;
            for(u32 j = 0; j < attack_count; ++j, ++attack)
            {
                if(!EntityIDEqual(health->parent_id, attack->parent_id) &&
                   attack->active)
                {
                    
                    v3 hit_sphere_to_health_sphere = {
                        attack->hit_sphere_position.x - health->position.x,
                        attack->hit_sphere_position.y - health->position.y,
                        attack->hit_sphere_position.z - health->position.z,
                    };
                    
                    f32 distance_squared = (hit_sphere_to_health_sphere.x*hit_sphere_to_health_sphere.x +
                                            hit_sphere_to_health_sphere.y*hit_sphere_to_health_sphere.y +
                                            hit_sphere_to_health_sphere.z*hit_sphere_to_health_sphere.z);
                    
                    f32 max_distance_squared = (attack->hit_sphere_radius+health->radius)*(attack->hit_sphere_radius+health->radius);
                    
                    if(distance_squared < max_distance_squared)
                    {
                        health->health -= 0.25f;
                        health->hurt_transition = 1.f;
                        
                        v3 hit_direction = V3Normalize(attack->hit_sphere_relative_position);
                        
                        v3 hit_velocity_vector = {
                            50.f*hit_direction.x,
                            50.f*hit_direction.y,
                            50.f*hit_direction.z,
                        };
                        
                        sphere->hit_velocity.x += hit_velocity_vector.x;
                        sphere->hit_velocity.y += hit_velocity_vector.y;
                        sphere->hit_velocity.z += hit_velocity_vector.z;
                        sphere->hit_velocity.y += 10.f;
                    }
                }
            }
        }
    }
}

internal void
CollideHealthAndSphereComponentsWithProjectiles(HealthComponent *health, SphereComponent *sphere,
                                                u32 count, ProjectileSet *projectiles,
                                                ParticleMaster *particles)
{
    for(u32 i = 0; i < count; ++i, ++health, ++sphere)
    {
        if(health->hurt_transition < 0.1f)
        {
            for(u32 j = 0; j < projectiles->projectile_count;)
            {
                v3 hit_sphere_to_health_sphere = {
                    projectiles->projectiles[j].position.x - health->position.x,
                    projectiles->projectiles[j].position.y - health->position.y,
                    projectiles->projectiles[j].position.z - health->position.z,
                };
                
                f32 distance_squared = (hit_sphere_to_health_sphere.x*hit_sphere_to_health_sphere.x +
                                        hit_sphere_to_health_sphere.y*hit_sphere_to_health_sphere.y +
                                        hit_sphere_to_health_sphere.z*hit_sphere_to_health_sphere.z);
                
                f32 max_distance_squared = (0.1f+health->radius)*(0.1f+health->radius);
                
                if(distance_squared < max_distance_squared)
                {
                    health->health -= 0.25f;
                    health->hurt_transition = 1.f;
                    
                    v3 hit_direction = V3Normalize(projectiles->projectiles[j].velocity);
                    
                    v3 hit_velocity_vector = {
                        50.f*hit_direction.x,
                        50.f*hit_direction.y,
                        50.f*hit_direction.z,
                    };
                    
                    sphere->hit_velocity.x += hit_velocity_vector.x;
                    sphere->hit_velocity.y += hit_velocity_vector.y;
                    sphere->hit_velocity.z += hit_velocity_vector.z;
                    sphere->hit_velocity.y += 10.f;
                    
                    for(u32 k = 0; k < 100; ++k)
                    {
                        ParticleSpawn(particles, PARTICLE_TYPE_strike, health->position, v3(RandomF32(-1, 1), RandomF32(-1, 1), RandomF32(-1, 1)));
                    }
                    
                    ProjectileSetRemove(projectiles, j);
                }
                else
                {
                    ++j;
                }
            }
        }
    }
}

internal void
TrackHealthComponentsToSphereComponents(HealthComponent *health, SphereComponent *sphere, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++health, ++sphere)
    {
        health->position = sphere->position;
        health->radius = sphere->radius;
    }
}

internal void
TrackSpriteComponentsToHealthComponents(SpriteComponent *sprite, HealthComponent *health, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++health, ++sprite)
    {
        sprite->tint = v4u(1);
        sprite->tint.g -= sprite->tint.g * health->hurt_transition;
        sprite->tint.b -= sprite->tint.b * health->hurt_transition;
    }
}

internal void
UpdateHealthComponents(HealthComponent *health, u32 n)
{
    for(u32 i = 0; i < n; ++i, ++health)
    {
        health->hurt_transition -= health->hurt_transition*4.f*app->delta_t;
    }
}

internal void
RenderSphereComponentsDebug(SphereComponent *sphere, u32 n)
{
    if(app->debug_draw_flags & DEBUG_DRAW_collision_volumes)
    {
        for(u32 i = 0; i < n; ++i, ++sphere)
        {
            RendererPushDebugSphere(&app->renderer, v4(1, 1, 1, 1), sphere->position, sphere->radius);
        }
    }
}

internal void
RenderAttackComponentsDebug(AttackComponent *attack, u32 n)
{
    if(app->debug_draw_flags & DEBUG_DRAW_hurt_volumes)
    {
        for(u32 i = 0; i < n; ++i, ++attack)
        {
            RendererPushDebugSphere(&app->renderer, v4(1, 0, 0, 1),
                                    v3(attack->hit_sphere_position.x + attack->hit_sphere_relative_position.x,
                                       attack->hit_sphere_position.y + attack->hit_sphere_relative_position.y,
                                       attack->hit_sphere_position.z + attack->hit_sphere_relative_position.z),
                                    attack->hit_sphere_radius);
        }
    }
}