typedef enum ParticleTypeIndex ParticleTypeIndex;
enum Index
{
    PARTICLE_TYPE_fire,
    PARTICLE_TYPE_strike,
    PARTICLE_TYPE_MAX
};

typedef struct ParticleType ParticleType;
struct ParticleType
{
    v4 source;
    b32 additive;
    u32 maximum;
};

global ParticleType global_particle_type_data[PARTICLE_TYPE_MAX] = {
    { { 96, 160, 16, 16 }, 1, 4096 },
    { { 96, 208, 16, 16 }, 1, 4096 },
};

typedef struct Particle Particle;
struct Particle
{
    v3 position;
    v3 velocity;
    f32 life;
};

typedef struct ParticleSet ParticleSet;
struct ParticleSet
{
    ParticleType *type;
    Particle *particles;
    u32 count;
    u32 max_count;
};

typedef struct ParticleMaster ParticleMaster;
struct ParticleMaster
{
    ParticleSet particle_sets[PARTICLE_TYPE_MAX];
};

internal ParticleMaster
ParticleMasterInit(MemoryArena *arena);

internal void
ParticleMasterUpdate(ParticleMaster *particles);

internal void
ParticleMasterRender(ParticleMaster *particles);

internal void
ParticleSpawn(ParticleMaster *particles, ParticleTypeIndex type, v3 position, v3 velocity);