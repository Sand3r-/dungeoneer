typedef struct SphereComponent SphereComponent;
struct SphereComponent
{
v3 position;
f32 radius;
v3 hit_velocity;
v3 force;
v3 velocity;
};

SphereComponent SphereComponentInit(v3 position, f32 radius)
{
SphereComponent sphere_component = {0};
sphere_component.position = position;
sphere_component.radius = radius;
return sphere_component;
}

typedef struct StaticFloatComponent StaticFloatComponent;
struct StaticFloatComponent
{
v3 position;
f32 float_sin_pos;
};

StaticFloatComponent StaticFloatComponentInit(v3 position)
{
StaticFloatComponent static_float_component = {0};
static_float_component.position = position;
return static_float_component;
}

typedef struct SpriteComponent SpriteComponent;
struct SpriteComponent
{
v3 position;
f32 scale;
Texture *texture;
v4 source;
i32 flags;
v4 tint;
f32 bob_sin_pos;
f32 bob_magnitude;
};

SpriteComponent SpriteComponentInit(v3 position, f32 scale, Texture *texture, v4 source, i32 flags, v4 tint)
{
SpriteComponent sprite_component = {0};
sprite_component.position = position;
sprite_component.scale = scale;
sprite_component.texture = texture;
sprite_component.source = source;
sprite_component.flags = flags;
sprite_component.tint = tint;
return sprite_component;
}

typedef struct AttackComponent AttackComponent;
struct AttackComponent
{
i32 type;
v3 position;
v3 hit_sphere_position;
v3 hit_sphere_relative_position;
f32 hit_sphere_max_radius;
f32 hit_sphere_radius;
v3 target_position;
f32 attack_time;
EntityID parent_id;
b32 active;
};

AttackComponent AttackComponentInit(i32 type, EntityID parent_id)
{
AttackComponent attack_component = {0};
attack_component.type = type;
attack_component.parent_id = parent_id;
return attack_component;
}

typedef struct WeaponComponent WeaponComponent;
struct WeaponComponent
{
Texture *texture;
v4 source;
f32 attack_transition;
f32 attack_angle;
v3 position;
f32 bob_sin_pos;
f32 bob_magnitude;
};

WeaponComponent WeaponComponentInit(Texture *texture, v4 source)
{
WeaponComponent weapon_component = {0};
weapon_component.texture = texture;
weapon_component.source = source;
return weapon_component;
}

typedef struct EnemyAIComponent EnemyAIComponent;
struct EnemyAIComponent
{
i32 type;
i32 state;
v3 position;
v3 movement;
f32 movement_time;
f32 time_to_movement;
b32 player_close;
};

EnemyAIComponent EnemyAIComponentInit(i32 type)
{
EnemyAIComponent enemy_aicomponent = {0};
enemy_aicomponent.type = type;
return enemy_aicomponent;
}

typedef struct HealthComponent HealthComponent;
struct HealthComponent
{
v3 position;
f32 radius;
f32 health;
EntityID parent_id;
f32 hurt_transition;
};

HealthComponent HealthComponentInit(v3 position, f32 radius, f32 health, EntityID parent_id)
{
HealthComponent health_component = {0};
health_component.position = position;
health_component.radius = radius;
health_component.health = health;
health_component.parent_id = parent_id;
return health_component;
}

typedef struct Map Map;
typedef struct Player Player;
typedef struct ParticleMaster ParticleMaster;
typedef struct ProjectileSet ProjectileSet;
#define COLLECTIBLE_MAX 64
#define COLLECTIBLE_TYPE_ID 44135
typedef struct CollectibleSet
{
u32 count;
u32 max_count;
u32 *id_to_index_table;
u32 *index_to_id_table;
u32 free_id_count;
u32 *free_id_list;
ItemType **item;
StaticFloatComponent *static_float;
SpriteComponent *sprite;
}
CollectibleSet;

void CollectibleSetInit(CollectibleSet *set, u32 maximum, MemoryArena *arena);
void CollectibleSetCleanUp(CollectibleSet *set);
u32 CollectibleSetGetNewID(CollectibleSet *set);
u32 CollectibleSetDevelopmentMatchesMapFileEntity(char *name);
int CollectibleSetAdd(CollectibleSet *set, v3 spawn_position, ItemType *item);
int CollectibleSetAddDefault(CollectibleSet *set, v3 spawn_position);
int CollectibleSetRemoveByIndex(CollectibleSet *set, u32 index);
int CollectibleSetRemoveByID(CollectibleSet *set, u32 id);
void CollectibleSetUpdate(CollectibleSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);
void CollectibleSetRender(CollectibleSet *set);

#define TREASURE_CHEST_MAX 64
#define TREASURE_CHEST_TYPE_ID 32967
typedef struct TreasureChestSet
{
u32 count;
u32 max_count;
u32 *id_to_index_table;
u32 *index_to_id_table;
u32 free_id_count;
u32 *free_id_list;
ItemType **item;
SphereComponent *sphere;
SpriteComponent *sprite;
}
TreasureChestSet;

void TreasureChestSetInit(TreasureChestSet *set, u32 maximum, MemoryArena *arena);
void TreasureChestSetCleanUp(TreasureChestSet *set);
u32 TreasureChestSetGetNewID(TreasureChestSet *set);
u32 TreasureChestSetDevelopmentMatchesMapFileEntity(char *name);
int TreasureChestSetAdd(TreasureChestSet *set, v3 spawn_position, ItemType *item);
int TreasureChestSetAddDefault(TreasureChestSet *set, v3 spawn_position);
int TreasureChestSetRemoveByIndex(TreasureChestSet *set, u32 index);
int TreasureChestSetRemoveByID(TreasureChestSet *set, u32 id);
void TreasureChestSetUpdate(TreasureChestSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);
void TreasureChestSetRender(TreasureChestSet *set);

#define TORCH_MAX 64
#define TORCH_TYPE_ID 55973
typedef struct TorchSet
{
u32 count;
u32 max_count;
u32 *id_to_index_table;
u32 *index_to_id_table;
u32 free_id_count;
u32 *free_id_list;
SphereComponent *sphere;
SpriteComponent *sprite;
}
TorchSet;

void TorchSetInit(TorchSet *set, u32 maximum, MemoryArena *arena);
void TorchSetCleanUp(TorchSet *set);
u32 TorchSetGetNewID(TorchSet *set);
u32 TorchSetDevelopmentMatchesMapFileEntity(char *name);
int TorchSetAdd(TorchSet *set, v3 spawn_position);
int TorchSetAddDefault(TorchSet *set, v3 spawn_position);
int TorchSetRemoveByIndex(TorchSet *set, u32 index);
int TorchSetRemoveByID(TorchSet *set, u32 id);
void TorchSetUpdate(TorchSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);
void TorchSetRender(TorchSet *set);

#define GOBLIN_MAX 64
#define GOBLIN_TYPE_ID 32736
typedef struct GoblinSet
{
u32 count;
u32 max_count;
u32 *id_to_index_table;
u32 *index_to_id_table;
u32 free_id_count;
u32 *free_id_list;
SphereComponent *sphere;
SpriteComponent *sprite;
EnemyAIComponent *enemy_ai;
HealthComponent *health;
AttackComponent *attack;
WeaponComponent *weapon;
}
GoblinSet;

void GoblinSetInit(GoblinSet *set, u32 maximum, MemoryArena *arena);
void GoblinSetCleanUp(GoblinSet *set);
u32 GoblinSetGetNewID(GoblinSet *set);
u32 GoblinSetDevelopmentMatchesMapFileEntity(char *name);
int GoblinSetAdd(GoblinSet *set, v3 spawn_position, SpriteComponent sprite);
int GoblinSetAddDefault(GoblinSet *set, v3 spawn_position);
int GoblinSetRemoveByIndex(GoblinSet *set, u32 index);
int GoblinSetRemoveByID(GoblinSet *set, u32 id);
void GoblinSetUpdate(GoblinSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);
void GoblinSetRender(GoblinSet *set);

#define DARK_WIZARD_MAX 64
#define DARK_WIZARD_TYPE_ID 15576
typedef struct DarkWizardSet
{
u32 count;
u32 max_count;
u32 *id_to_index_table;
u32 *index_to_id_table;
u32 free_id_count;
u32 *free_id_list;
SphereComponent *sphere;
SpriteComponent *sprite;
EnemyAIComponent *enemy_ai;
HealthComponent *health;
AttackComponent *attack;
}
DarkWizardSet;

void DarkWizardSetInit(DarkWizardSet *set, u32 maximum, MemoryArena *arena);
void DarkWizardSetCleanUp(DarkWizardSet *set);
u32 DarkWizardSetGetNewID(DarkWizardSet *set);
u32 DarkWizardSetDevelopmentMatchesMapFileEntity(char *name);
int DarkWizardSetAdd(DarkWizardSet *set, v3 spawn_position, SpriteComponent sprite);
int DarkWizardSetAddDefault(DarkWizardSet *set, v3 spawn_position);
int DarkWizardSetRemoveByIndex(DarkWizardSet *set, u32 index);
int DarkWizardSetRemoveByID(DarkWizardSet *set, u32 id);
void DarkWizardSetUpdate(DarkWizardSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);
void DarkWizardSetRender(DarkWizardSet *set);

#define KNIGHT_MAX 64
#define KNIGHT_TYPE_ID 52010
typedef struct KnightSet
{
u32 count;
u32 max_count;
u32 *id_to_index_table;
u32 *index_to_id_table;
u32 free_id_count;
u32 *free_id_list;
SphereComponent *sphere;
SpriteComponent *sprite;
EnemyAIComponent *enemy_ai;
HealthComponent *health;
AttackComponent *attack;
WeaponComponent *weapon;
}
KnightSet;

void KnightSetInit(KnightSet *set, u32 maximum, MemoryArena *arena);
void KnightSetCleanUp(KnightSet *set);
u32 KnightSetGetNewID(KnightSet *set);
u32 KnightSetDevelopmentMatchesMapFileEntity(char *name);
int KnightSetAdd(KnightSet *set, v3 spawn_position, SpriteComponent sprite);
int KnightSetAddDefault(KnightSet *set, v3 spawn_position);
int KnightSetRemoveByIndex(KnightSet *set, u32 index);
int KnightSetRemoveByID(KnightSet *set, u32 id);
void KnightSetUpdate(KnightSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);
void KnightSetRender(KnightSet *set);

#define CRATE_MAX 64
#define CRATE_TYPE_ID 39188
typedef struct CrateSet
{
u32 count;
u32 max_count;
u32 *id_to_index_table;
u32 *index_to_id_table;
u32 free_id_count;
u32 *free_id_list;
ItemType **item;
SphereComponent *sphere;
SpriteComponent *sprite;
HealthComponent *health;
}
CrateSet;

void CrateSetInit(CrateSet *set, u32 maximum, MemoryArena *arena);
void CrateSetCleanUp(CrateSet *set);
u32 CrateSetGetNewID(CrateSet *set);
u32 CrateSetDevelopmentMatchesMapFileEntity(char *name);
int CrateSetAdd(CrateSet *set, v3 spawn_position, ItemType *item);
int CrateSetAddDefault(CrateSet *set, v3 spawn_position);
int CrateSetRemoveByIndex(CrateSet *set, u32 index);
int CrateSetRemoveByID(CrateSet *set, u32 id);
void CrateSetUpdate(CrateSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);
void CrateSetRender(CrateSet *set);

typedef struct EntitySets
{
CollectibleSet collectible_set;
TreasureChestSet treasure_chest_set;
TorchSet torch_set;
GoblinSet goblin_set;
DarkWizardSet dark_wizard_set;
KnightSet knight_set;
CrateSet crate_set;
}
EntitySets;

void EntitySetsInit(EntitySets *entities, MemoryArena *arena);
void EntitySetsCleanUp(EntitySets *entities);
typedef struct ParticleMaster ParticleMaster;
void EntitySetsDoDefaultUpdate(EntitySets *entities, Map *map, Player *player, ProjectileSet *projectiles, ParticleMaster *particles);
void EntitySetsRender(EntitySets *entities);
void EntitySetsRemoveByID(EntitySets *entities, EntityID id);
