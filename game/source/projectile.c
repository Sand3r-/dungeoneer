internal void
ProjectileSetInit(ProjectileSet *set, MemoryArena *arena, u32 max_projectiles)
{
    set->projectile_count = 0;
    set->max_projectile_count = max_projectiles;
    set->projectiles = MemoryArenaAllocate(arena, max_projectiles*sizeof(Projectile));
}

internal void
ProjectileSetUpdate(ProjectileSet *set, Map *map, ParticleMaster *particles)
{
    for(u32 i = 0; i < set->projectile_count;)
    {
        Projectile *projectile = set->projectiles + i;
        
        projectile->position.x += projectile->velocity.x * app->delta_t;
        projectile->position.y += projectile->velocity.y * app->delta_t;
        projectile->position.z += projectile->velocity.z * app->delta_t;
        projectile->seconds_to_death -= app->delta_t;
        
        if(MapGetTile(map, iv2((i32)(projectile->position.x / TILE_SIZE),
                               (i32)(projectile->position.z / TILE_SIZE))).flags & MAP_TILE_WALL)
        {
            projectile->seconds_to_death = 0.f;
            
            for(u32 k = 0; k < 100; ++k)
            {
                ParticleSpawn(particles, PARTICLE_TYPE_strike, projectile->position, v3(RandomF32(-2, 2), RandomF32(-2, 2), RandomF32(-2, 2)));
            }
            
        }
        
        if(projectile->seconds_to_death <= 0.f)
        {
            ProjectileSetRemove(set, i);
        }
        else
        {
            ++i;
        }
    }
}

internal void
ProjectileSetRender(ProjectileSet *set, ParticleMaster *particles)
{
    for(u32 i = 0; i < set->projectile_count; ++i)
    {
        Projectile *projectile = set->projectiles + i;
        
        switch(projectile->type)
        {
            case PROJECTILE_TYPE_fire:
            {
                ParticleSpawn(particles, PARTICLE_TYPE_fire, projectile->position,
                              v3(RandomF32(-1.f, 1.f), RandomF32(-1.f, 1.f), RandomF32(-1.f, 1.f)));
                RendererPushPointLight(&app->renderer, projectile->position,
                                       v3(1.f, 0.75f, 0.6f), 4.f, 2.f);
                break;
            }
            default: break;
        }
    }
}

internal void
ProjectileSetRemove(ProjectileSet *set, u32 index)
{
    if(set->projectile_count > 0 && index != set->projectile_count-1)
    {
        MemoryCopy(set->projectiles+index, set->projectiles+(set->projectile_count-1),
                   sizeof(Projectile));
    }
    --set->projectile_count;
}

internal void
ProjectileSpawn(ProjectileSet *set, ProjectileTypeIndex type, v3 position, v3 velocity, i32 damage_flags, f32 strength)
{
    if(set->projectile_count < set->max_projectile_count)
    {
        u32 i = set->projectile_count++;
        set->projectiles[i].type = type;
        set->projectiles[i].position = position;
        set->projectiles[i].velocity = velocity;
        set->projectiles[i].seconds_to_death = 2.f;
        set->projectiles[i].strength = strength;
        set->projectiles[i].damage_flags = damage_flags;
    }
}
