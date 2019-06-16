typedef enum ProjectileTypeIndex ProjectileTypeIndex;
enum ProjectileTypeIndex
{
    PROJECTILE_TYPE_fire,
};

typedef struct Projectile Projectile;
struct Projectile
{
    ProjectileTypeIndex type;
    v3 position;
    v3 velocity;
    f32 seconds_to_death;
    f32 strength;
    i32 damage_flags;
};

typedef struct ProjectileSet ProjectileSet;
struct ProjectileSet
{
    u32 projectile_count;
    u32 max_projectile_count;
    Projectile *projectiles;
};

internal void
ProjectileSetInit(ProjectileSet *set, MemoryArena *arena, u32 max_projectiles);

internal void
ProjectileSetUpdate(ProjectileSet *set, Map *map, ParticleMaster *particles);

internal void
ProjectileSetRender(ProjectileSet *set, ParticleMaster *particles);

internal void
ProjectileSetRemove(ProjectileSet *set, u32 index);

internal void
ProjectileSpawn(ProjectileSet *set, ProjectileTypeIndex type, v3 position, v3 velocity, i32 damage_flags, f32 strength);
