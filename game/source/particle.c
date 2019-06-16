
internal ParticleSet
ParticleSetInit(MemoryArena *arena, ParticleType *type)
{
    ParticleSet set = {0};
    set.count = 0;
    set.max_count = type->maximum;
    set.particles = MemoryArenaAllocate(arena, sizeof(Particle)*type->maximum);
    set.type = type;
    return set;
}

internal void
ParticleSetUpdate(ParticleSet *set)
{
    for(i32 i = 0; i < (i32)set->count; ++i)
    {
        set->particles[i].position.x += set->particles[i].velocity.x*app->delta_t;
        set->particles[i].position.y += set->particles[i].velocity.y*app->delta_t;
        set->particles[i].position.z += set->particles[i].velocity.z*app->delta_t;
        set->particles[i].life -= app->delta_t;
        if(set->particles[i].life <= 0.f)
        {
            --set->count;
            if(i != set->count)
            {
                MemoryCopy(set->particles+i, set->particles+set->count, sizeof(Particle));
            }
            --i;
        }
    }
}

internal void
ParticleSetRender(ParticleSet *set)
{
    for(u32 i = 0; i < set->count; ++i)
    {
        v3 center = set->particles[i].position;
        f32 scale = 0.4f;
        v3 p1 = { center.x - scale, center.y - scale, center.z };
        v3 p2 = { center.x - scale, center.y + scale, center.z - 0.2f };
        v3 p3 = { center.x + scale, center.y - scale, center.z };
        v3 p4 = { center.x + scale, center.y + scale, center.z - 0.2f };
        RendererPushTexture(&app->renderer, &app->art,
                            RENDERER_NO_SHADOW |
                            RENDERER_NO_DEPTH_WRITE |
                            (set->type->additive ? RENDERER_ADDITIVE_BLEND : 0),
                            set->type->source,
                            p1, p2, p3, p4,
                            v4u(set->particles[i].life));
    }
}

internal ParticleMaster
ParticleMasterInit(MemoryArena *arena)
{
    ParticleMaster master = {0};
    
    for(u32 i = 0; i < PARTICLE_TYPE_MAX; ++i)
    {
        ParticleType *type = global_particle_type_data + i;
        master.particle_sets[i] = ParticleSetInit(arena, type);
    }
    
    return master;
}

internal void
ParticleMasterUpdate(ParticleMaster *particles)
{
    for(u32 i = 0; i < PARTICLE_TYPE_MAX; ++i)
    {
        ParticleSetUpdate(particles->particle_sets+i);
    }
}

internal void
ParticleMasterRender(ParticleMaster *particles)
{
    for(u32 i = 0; i < PARTICLE_TYPE_MAX; ++i)
    {
        ParticleSetRender(particles->particle_sets+i);
    }
}

internal void
ParticleSpawn(ParticleMaster *particles, ParticleTypeIndex type, v3 position, v3 velocity)
{
    HardAssert(type >= (ParticleTypeIndex)0 && type < PARTICLE_TYPE_MAX);
    ParticleSet *set = particles->particle_sets + type;
    if(set->count < set->max_count)
    {
        u32 i = set->count++;
        set->particles[i].position = position;
        set->particles[i].velocity = velocity;
        set->particles[i].life = 1.f;
    }
}