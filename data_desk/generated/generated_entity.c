void EntitySetsInit(EntitySets *entities, MemoryArena *arena)
{
CollectibleSetInit(&entities->collectible_set, COLLECTIBLE_MAX, arena);
TreasureChestSetInit(&entities->treasure_chest_set, TREASURE_CHEST_MAX, arena);
TorchSetInit(&entities->torch_set, TORCH_MAX, arena);
GoblinSetInit(&entities->goblin_set, GOBLIN_MAX, arena);
DarkWizardSetInit(&entities->dark_wizard_set, DARK_WIZARD_MAX, arena);
KnightSetInit(&entities->knight_set, KNIGHT_MAX, arena);
CrateSetInit(&entities->crate_set, CRATE_MAX, arena);
}

void EntitySetsCleanUp(EntitySets *entities)
{
CollectibleSetCleanUp(&entities->collectible_set);
TreasureChestSetCleanUp(&entities->treasure_chest_set);
TorchSetCleanUp(&entities->torch_set);
GoblinSetCleanUp(&entities->goblin_set);
DarkWizardSetCleanUp(&entities->dark_wizard_set);
KnightSetCleanUp(&entities->knight_set);
CrateSetCleanUp(&entities->crate_set);
}

internal void CollideHealthAndSphereComponentsWithAttackComponents(HealthComponent *health, SphereComponent *sphere, u32 count, AttackComponent *attack, u32 attack_count, ParticleMaster *particles);
void EntitySetsDoDefaultUpdate(EntitySets *entities, Map *map, Player *player, ProjectileSet *projectiles, ParticleMaster *particles)
{
CollectibleSetUpdate(&entities->collectible_set, map, particles, player, projectiles);
TreasureChestSetUpdate(&entities->treasure_chest_set, map, particles, player, projectiles);
TorchSetUpdate(&entities->torch_set, map, particles, player, projectiles);
GoblinSetUpdate(&entities->goblin_set, map, particles, player, projectiles);
DarkWizardSetUpdate(&entities->dark_wizard_set, map, particles, player, projectiles);
KnightSetUpdate(&entities->knight_set, map, particles, player, projectiles);
CrateSetUpdate(&entities->crate_set, map, particles, player, projectiles);
struct
{
AttackComponent *attack; u32 count;
}
attack_set_list[] = {
{ &player->attack, 1, },
{ entities->goblin_set.attack, entities->goblin_set.count, },
{ entities->dark_wizard_set.attack, entities->dark_wizard_set.count, },
{ entities->knight_set.attack, entities->knight_set.count, },
};

for(u32 i = 0; i < ArrayCount(attack_set_list); ++i)
{
CollideHealthAndSphereComponentsWithAttackComponents(entities->goblin_set.health, entities->goblin_set.sphere, entities->goblin_set.count, attack_set_list[i].attack, attack_set_list[i].count, particles);
}

CollideHealthAndSphereComponentsWithProjectiles(entities->goblin_set.health, entities->goblin_set.sphere, entities->goblin_set.count, projectiles, particles);
CollideHealthAndSphereComponentsWithAttackComponents(&player->health, &player->sphere, 1, entities->goblin_set.attack, entities->goblin_set.count, particles);
for(u32 i = 0; i < ArrayCount(attack_set_list); ++i)
{
CollideHealthAndSphereComponentsWithAttackComponents(entities->dark_wizard_set.health, entities->dark_wizard_set.sphere, entities->dark_wizard_set.count, attack_set_list[i].attack, attack_set_list[i].count, particles);
}

CollideHealthAndSphereComponentsWithProjectiles(entities->dark_wizard_set.health, entities->dark_wizard_set.sphere, entities->dark_wizard_set.count, projectiles, particles);
CollideHealthAndSphereComponentsWithAttackComponents(&player->health, &player->sphere, 1, entities->dark_wizard_set.attack, entities->dark_wizard_set.count, particles);
for(u32 i = 0; i < ArrayCount(attack_set_list); ++i)
{
CollideHealthAndSphereComponentsWithAttackComponents(entities->knight_set.health, entities->knight_set.sphere, entities->knight_set.count, attack_set_list[i].attack, attack_set_list[i].count, particles);
}

CollideHealthAndSphereComponentsWithProjectiles(entities->knight_set.health, entities->knight_set.sphere, entities->knight_set.count, projectiles, particles);
CollideHealthAndSphereComponentsWithAttackComponents(&player->health, &player->sphere, 1, entities->knight_set.attack, entities->knight_set.count, particles);
for(u32 i = 0; i < ArrayCount(attack_set_list); ++i)
{
CollideHealthAndSphereComponentsWithAttackComponents(entities->crate_set.health, entities->crate_set.sphere, entities->crate_set.count, attack_set_list[i].attack, attack_set_list[i].count, particles);
}

CollideHealthAndSphereComponentsWithProjectiles(entities->crate_set.health, entities->crate_set.sphere, entities->crate_set.count, projectiles, particles);
}

void EntitySetsRender(EntitySets *entities)
{
CollectibleSetRender(&entities->collectible_set);
TreasureChestSetRender(&entities->treasure_chest_set);
TorchSetRender(&entities->torch_set);
GoblinSetRender(&entities->goblin_set);
DarkWizardSetRender(&entities->dark_wizard_set);
KnightSetRender(&entities->knight_set);
CrateSetRender(&entities->crate_set);
}

void EntitySetsRemoveByID(EntitySets *entities, EntityID id)
{
if(id.type == COLLECTIBLE_TYPE_ID)
{
CollectibleSetRemoveByID(&entities->collectible_set, entities->collectible_set.id_to_index_table[id.instance_id]);
}
else
if(id.type == TREASURE_CHEST_TYPE_ID)
{
TreasureChestSetRemoveByID(&entities->treasure_chest_set, entities->treasure_chest_set.id_to_index_table[id.instance_id]);
}
else
if(id.type == TORCH_TYPE_ID)
{
TorchSetRemoveByID(&entities->torch_set, entities->torch_set.id_to_index_table[id.instance_id]);
}
else
if(id.type == GOBLIN_TYPE_ID)
{
GoblinSetRemoveByID(&entities->goblin_set, entities->goblin_set.id_to_index_table[id.instance_id]);
}
else
if(id.type == DARK_WIZARD_TYPE_ID)
{
DarkWizardSetRemoveByID(&entities->dark_wizard_set, entities->dark_wizard_set.id_to_index_table[id.instance_id]);
}
else
if(id.type == KNIGHT_TYPE_ID)
{
KnightSetRemoveByID(&entities->knight_set, entities->knight_set.id_to_index_table[id.instance_id]);
}
else
if(id.type == CRATE_TYPE_ID)
{
CrateSetRemoveByID(&entities->crate_set, entities->crate_set.id_to_index_table[id.instance_id]);
}
else
{}
}

void CollectibleSetInit(CollectibleSet *set, u32 maximum, MemoryArena *arena)
{
set->count = 0;
set->max_count = maximum;
set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->free_id_count = maximum;
set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);

for(u32 i = 0; i < maximum; ++i)
{
set->free_id_list[i] = maximum - i - 1;
}

set->item = (ItemType **)MemoryArenaAllocate(arena, sizeof(set->item[0]) * maximum);
set->static_float = (StaticFloatComponent *)MemoryArenaAllocate(arena, sizeof(set->static_float[0]) * maximum);
set->sprite = (SpriteComponent *)MemoryArenaAllocate(arena, sizeof(set->sprite[0]) * maximum);
}

void CollectibleSetCleanUp(CollectibleSet *set)
{
}

u32 CollectibleSetGetNewID(CollectibleSet *set)
{
u32 id = 0;
if(set->free_id_count > 0)
{
--set->free_id_count;
id = set->free_id_list[set->free_id_count];
}
return id;
}

u32 CollectibleSetDevelopmentMatchesMapFileEntity(char *name)
{
return CStringMatchCaseInsensitive(name, "Collectible");
}

int CollectibleSetInitInstanceByIndex(CollectibleSet *set, u32 index, v3 spawn_position, ItemType *item)
{
int success = 0;
if(index < set->max_count)
{
success = 1;
EntityID id = { (u16)COLLECTIBLE_TYPE_ID, (u16)index, };
set->item[index] = item;
StaticFloatComponent static_float = StaticFloatComponentInit(spawn_position);
set->static_float[index] = static_float;
SpriteComponent sprite = SpriteComponentInit(spawn_position, 1.f, &app->art, item->source, 0, v4u(1));
set->sprite[index] = sprite;
}
return success;
}

int CollectibleSetAdd(CollectibleSet *set, v3 spawn_position, ItemType *item)
{
int success = 0;
if(set->count < set->max_count)
{
success = 1;
u32 index = set->count++;
u32 id = CollectibleSetGetNewID(set);
set->index_to_id_table[index] = id;
set->id_to_index_table[id] = index;
CollectibleSetInitInstanceByIndex(set, index, spawn_position, item);
}
return success;
}

int CollectibleSetAddDefault(CollectibleSet *set, v3 spawn_position)
{
ItemType *item = global_item_type_data+0;return CollectibleSetAdd(set, spawn_position, item);
}

int CollectibleSetRemoveByIndex(CollectibleSet *set, u32 index)
{
int success = 0;

if(index >= 0 && index < set->count)
{
success = 1;
if(index != --set->count)
{
set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];
u32 replacement_index = set->count;
u32 replacement_id = set->index_to_id_table[replacement_index];
set->id_to_index_table[replacement_id] = replacement_index;
set->index_to_id_table[replacement_index] = replacement_id;
set->item[index] = set->item[replacement_index];
set->static_float[index] = set->static_float[replacement_index];
set->sprite[index] = set->sprite[replacement_index];
}

}

return success;
}

int CollectibleSetRemoveByID(CollectibleSet *set, u32 id)
{
return CollectibleSetRemoveByIndex(set, set->id_to_index_table[id]);
}

void CollectibleSetUpdate(CollectibleSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)
{
UpdateStaticFloatComponents(set->static_float, map, set->count);
TrackSpriteComponentsToStaticFloatComponents(set->sprite, set->static_float, set->count);
UpdateSpriteComponents(set->sprite, set->count);
}

void CollectibleSetRender(CollectibleSet *set)
{
RenderSpriteComponents(set->sprite, set->count);
}

void TreasureChestSetInit(TreasureChestSet *set, u32 maximum, MemoryArena *arena)
{
set->count = 0;
set->max_count = maximum;
set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->free_id_count = maximum;
set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);

for(u32 i = 0; i < maximum; ++i)
{
set->free_id_list[i] = maximum - i - 1;
}

set->item = (ItemType **)MemoryArenaAllocate(arena, sizeof(set->item[0]) * maximum);
set->sphere = (SphereComponent *)MemoryArenaAllocate(arena, sizeof(set->sphere[0]) * maximum);
set->sprite = (SpriteComponent *)MemoryArenaAllocate(arena, sizeof(set->sprite[0]) * maximum);
}

void TreasureChestSetCleanUp(TreasureChestSet *set)
{
}

u32 TreasureChestSetGetNewID(TreasureChestSet *set)
{
u32 id = 0;
if(set->free_id_count > 0)
{
--set->free_id_count;
id = set->free_id_list[set->free_id_count];
}
return id;
}

u32 TreasureChestSetDevelopmentMatchesMapFileEntity(char *name)
{
return CStringMatchCaseInsensitive(name, "Treasure Chest");
}

int TreasureChestSetInitInstanceByIndex(TreasureChestSet *set, u32 index, v3 spawn_position, ItemType *item)
{
int success = 0;
if(index < set->max_count)
{
success = 1;
EntityID id = { (u16)TREASURE_CHEST_TYPE_ID, (u16)index, };
set->item[index] = item;
SphereComponent sphere = SphereComponentInit(spawn_position, 1.f);
set->sphere[index] = sphere;
SpriteComponent sprite = SpriteComponentInit(spawn_position, 1.f, &app->art,  v4(224, 176, 16, 16), 0, v4u(1));
set->sprite[index] = sprite;
}
return success;
}

int TreasureChestSetAdd(TreasureChestSet *set, v3 spawn_position, ItemType *item)
{
int success = 0;
if(set->count < set->max_count)
{
success = 1;
u32 index = set->count++;
u32 id = TreasureChestSetGetNewID(set);
set->index_to_id_table[index] = id;
set->id_to_index_table[id] = index;
TreasureChestSetInitInstanceByIndex(set, index, spawn_position, item);
}
return success;
}

int TreasureChestSetAddDefault(TreasureChestSet *set, v3 spawn_position)
{
ItemType *item = global_item_type_data+0;return TreasureChestSetAdd(set, spawn_position, item);
}

int TreasureChestSetRemoveByIndex(TreasureChestSet *set, u32 index)
{
int success = 0;

if(index >= 0 && index < set->count)
{
success = 1;
if(index != --set->count)
{
set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];
u32 replacement_index = set->count;
u32 replacement_id = set->index_to_id_table[replacement_index];
set->id_to_index_table[replacement_id] = replacement_index;
set->index_to_id_table[replacement_index] = replacement_id;
set->item[index] = set->item[replacement_index];
set->sphere[index] = set->sphere[replacement_index];
set->sprite[index] = set->sprite[replacement_index];
}

}

return success;
}

int TreasureChestSetRemoveByID(TreasureChestSet *set, u32 id)
{
return TreasureChestSetRemoveByIndex(set, set->id_to_index_table[id]);
}

void TreasureChestSetUpdate(TreasureChestSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)
{
UpdateSphereComponents(set->sphere, map, set->count);
TrackSpriteComponentsToSphereComponents(set->sprite, set->sphere, set->count, map);
UpdateSpriteComponents(set->sprite, set->count);
}

void TreasureChestSetRender(TreasureChestSet *set)
{
RenderSpriteComponents(set->sprite, set->count);
RenderSphereComponentsDebug(set->sphere, set->count);
}

void TorchSetInit(TorchSet *set, u32 maximum, MemoryArena *arena)
{
set->count = 0;
set->max_count = maximum;
set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->free_id_count = maximum;
set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);

for(u32 i = 0; i < maximum; ++i)
{
set->free_id_list[i] = maximum - i - 1;
}

set->sphere = (SphereComponent *)MemoryArenaAllocate(arena, sizeof(set->sphere[0]) * maximum);
set->sprite = (SpriteComponent *)MemoryArenaAllocate(arena, sizeof(set->sprite[0]) * maximum);
}

void TorchSetCleanUp(TorchSet *set)
{
}

u32 TorchSetGetNewID(TorchSet *set)
{
u32 id = 0;
if(set->free_id_count > 0)
{
--set->free_id_count;
id = set->free_id_list[set->free_id_count];
}
return id;
}

u32 TorchSetDevelopmentMatchesMapFileEntity(char *name)
{
return CStringMatchCaseInsensitive(name, "Torch");
}

int TorchSetInitInstanceByIndex(TorchSet *set, u32 index, v3 spawn_position)
{
int success = 0;
if(index < set->max_count)
{
success = 1;
EntityID id = { (u16)TORCH_TYPE_ID, (u16)index, };
SphereComponent sphere = SphereComponentInit(spawn_position, 1.f);
set->sphere[index] = sphere;
SpriteComponent sprite = SpriteComponentInit(spawn_position, 1.f, &app->art, v4(112, 160, 16, 16), 0, v4u(1));
set->sprite[index] = sprite;
}
return success;
}

int TorchSetAdd(TorchSet *set, v3 spawn_position)
{
int success = 0;
if(set->count < set->max_count)
{
success = 1;
u32 index = set->count++;
u32 id = TorchSetGetNewID(set);
set->index_to_id_table[index] = id;
set->id_to_index_table[id] = index;
TorchSetInitInstanceByIndex(set, index, spawn_position);
}
return success;
}

int TorchSetAddDefault(TorchSet *set, v3 spawn_position)
{
return TorchSetAdd(set, spawn_position);
}

int TorchSetRemoveByIndex(TorchSet *set, u32 index)
{
int success = 0;

if(index >= 0 && index < set->count)
{
success = 1;
if(index != --set->count)
{
set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];
u32 replacement_index = set->count;
u32 replacement_id = set->index_to_id_table[replacement_index];
set->id_to_index_table[replacement_id] = replacement_index;
set->index_to_id_table[replacement_index] = replacement_id;
set->sphere[index] = set->sphere[replacement_index];
set->sprite[index] = set->sprite[replacement_index];
}

}

return success;
}

int TorchSetRemoveByID(TorchSet *set, u32 id)
{
return TorchSetRemoveByIndex(set, set->id_to_index_table[id]);
}

void TorchSetUpdate(TorchSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)
{
UpdateSphereComponents(set->sphere, map, set->count);
TrackSpriteComponentsToSphereComponents(set->sprite, set->sphere, set->count, map);
UpdateSpriteComponents(set->sprite, set->count);
}

void TorchSetRender(TorchSet *set)
{
RenderSpriteComponents(set->sprite, set->count);
RenderSphereComponentsDebug(set->sphere, set->count);
}

void GoblinSetInit(GoblinSet *set, u32 maximum, MemoryArena *arena)
{
set->count = 0;
set->max_count = maximum;
set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->free_id_count = maximum;
set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);

for(u32 i = 0; i < maximum; ++i)
{
set->free_id_list[i] = maximum - i - 1;
}

set->sphere = (SphereComponent *)MemoryArenaAllocate(arena, sizeof(set->sphere[0]) * maximum);
set->sprite = (SpriteComponent *)MemoryArenaAllocate(arena, sizeof(set->sprite[0]) * maximum);
set->enemy_ai = (EnemyAIComponent *)MemoryArenaAllocate(arena, sizeof(set->enemy_ai[0]) * maximum);
set->health = (HealthComponent *)MemoryArenaAllocate(arena, sizeof(set->health[0]) * maximum);
set->attack = (AttackComponent *)MemoryArenaAllocate(arena, sizeof(set->attack[0]) * maximum);
set->weapon = (WeaponComponent *)MemoryArenaAllocate(arena, sizeof(set->weapon[0]) * maximum);
}

void GoblinSetCleanUp(GoblinSet *set)
{
}

u32 GoblinSetGetNewID(GoblinSet *set)
{
u32 id = 0;
if(set->free_id_count > 0)
{
--set->free_id_count;
id = set->free_id_list[set->free_id_count];
}
return id;
}

u32 GoblinSetDevelopmentMatchesMapFileEntity(char *name)
{
return CStringMatchCaseInsensitive(name, "Goblin");
}

int GoblinSetInitInstanceByIndex(GoblinSet *set, u32 index, v3 spawn_position, SpriteComponent sprite)
{
int success = 0;
if(index < set->max_count)
{
success = 1;
EntityID id = { (u16)GOBLIN_TYPE_ID, (u16)index, };
SphereComponent sphere = SphereComponentInit(spawn_position, 1.f);
set->sphere[index] = sphere;
set->sprite[index] = sprite;
EnemyAIComponent enemy_ai = EnemyAIComponentInit(ENEMY_AI_TYPE_melee);
set->enemy_ai[index] = enemy_ai;
HealthComponent health = HealthComponentInit(spawn_position, 1.f, 1.f, id);
set->health[index] = health;
AttackComponent attack = AttackComponentInit(ATTACK_TYPE_melee, id);
set->attack[index] = attack;
WeaponComponent weapon = WeaponComponentInitFromItemType(global_item_type_data + ITEM_TYPE_dagger);
set->weapon[index] = weapon;
}
return success;
}

int GoblinSetAdd(GoblinSet *set, v3 spawn_position, SpriteComponent sprite)
{
int success = 0;
if(set->count < set->max_count)
{
success = 1;
u32 index = set->count++;
u32 id = GoblinSetGetNewID(set);
set->index_to_id_table[index] = id;
set->id_to_index_table[id] = index;
GoblinSetInitInstanceByIndex(set, index, spawn_position, sprite);
}
return success;
}

int GoblinSetAddDefault(GoblinSet *set, v3 spawn_position)
{
SpriteComponent sprite = SpriteComponentInit(spawn_position, 1.f, &app->art, v4(32, 160, 16, 16), 0, v4u(1));return GoblinSetAdd(set, spawn_position, sprite);
}

int GoblinSetRemoveByIndex(GoblinSet *set, u32 index)
{
int success = 0;

if(index >= 0 && index < set->count)
{
success = 1;
if(index != --set->count)
{
set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];
u32 replacement_index = set->count;
u32 replacement_id = set->index_to_id_table[replacement_index];
set->id_to_index_table[replacement_id] = replacement_index;
set->index_to_id_table[replacement_index] = replacement_id;
set->sphere[index] = set->sphere[replacement_index];
set->sprite[index] = set->sprite[replacement_index];
set->enemy_ai[index] = set->enemy_ai[replacement_index];
set->health[index] = set->health[replacement_index];
set->attack[index] = set->attack[replacement_index];
set->weapon[index] = set->weapon[replacement_index];
}

}

return success;
}

int GoblinSetRemoveByID(GoblinSet *set, u32 id)
{
return GoblinSetRemoveByIndex(set, set->id_to_index_table[id]);
}

void GoblinSetUpdate(GoblinSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)
{
UpdateSphereComponents(set->sphere, map, set->count);
TrackSpriteComponentsToSphereComponents(set->sprite, set->sphere, set->count, map);
TrackWeaponComponentsToSphereComponents(set->weapon, set->sphere, set->count);
UpdateSpriteComponents(set->sprite, set->count);
UpdateEnemyAI(set->enemy_ai, set->attack, set->count, player);
TrackEnemyAIComponentsToSphereComponents(set->enemy_ai, set->sphere, set->count);
TrackSphereComponentsToEnemyAIComponents(set->sphere, set->enemy_ai, set->count);
UpdateAttackComponents(set->attack, set->count, projectiles);
TrackAttackComponentsToSphereComponents(set->attack, set->sphere, set->count);
UpdateHealthComponents(set->health, set->count);
TrackHealthComponentsToSphereComponents(set->health, set->sphere, set->count);
TrackSpriteComponentsToHealthComponents(set->sprite, set->health, set->count);
for(u32 i = 0; i < set->count;)
{
    if(set->health[i].health <= 0.f)
{
        for(u32 j = 0; j < 100; ++j)
{
            ParticleSpawn(particles, PARTICLE_TYPE_strike, set->health[i].position, v3(RandomF32(-1, 1), RandomF32(-1, 1), RandomF32(-1, 1)));
        }
        GoblinSetRemoveByIndex(set, i);
    }
    else
{
        ++i;
    }
}

}

void GoblinSetRender(GoblinSet *set)
{
RenderSpriteComponents(set->sprite, set->count);
RenderWeaponComponents(set->weapon, set->count);
RenderSphereComponentsDebug(set->sphere, set->count);
RenderAttackComponentsDebug(set->attack, set->count);
}

void DarkWizardSetInit(DarkWizardSet *set, u32 maximum, MemoryArena *arena)
{
set->count = 0;
set->max_count = maximum;
set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->free_id_count = maximum;
set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);

for(u32 i = 0; i < maximum; ++i)
{
set->free_id_list[i] = maximum - i - 1;
}

set->sphere = (SphereComponent *)MemoryArenaAllocate(arena, sizeof(set->sphere[0]) * maximum);
set->sprite = (SpriteComponent *)MemoryArenaAllocate(arena, sizeof(set->sprite[0]) * maximum);
set->enemy_ai = (EnemyAIComponent *)MemoryArenaAllocate(arena, sizeof(set->enemy_ai[0]) * maximum);
set->health = (HealthComponent *)MemoryArenaAllocate(arena, sizeof(set->health[0]) * maximum);
set->attack = (AttackComponent *)MemoryArenaAllocate(arena, sizeof(set->attack[0]) * maximum);
}

void DarkWizardSetCleanUp(DarkWizardSet *set)
{
}

u32 DarkWizardSetGetNewID(DarkWizardSet *set)
{
u32 id = 0;
if(set->free_id_count > 0)
{
--set->free_id_count;
id = set->free_id_list[set->free_id_count];
}
return id;
}

u32 DarkWizardSetDevelopmentMatchesMapFileEntity(char *name)
{
return CStringMatchCaseInsensitive(name, "Dark Wizard");
}

int DarkWizardSetInitInstanceByIndex(DarkWizardSet *set, u32 index, v3 spawn_position, SpriteComponent sprite)
{
int success = 0;
if(index < set->max_count)
{
success = 1;
EntityID id = { (u16)DARK_WIZARD_TYPE_ID, (u16)index, };
SphereComponent sphere = SphereComponentInit(spawn_position, 1.f);
set->sphere[index] = sphere;
set->sprite[index] = sprite;
EnemyAIComponent enemy_ai = EnemyAIComponentInit(ENEMY_AI_TYPE_ranged);
set->enemy_ai[index] = enemy_ai;
HealthComponent health = HealthComponentInit(spawn_position, 1.f, 1.f, id);
set->health[index] = health;
AttackComponent attack = AttackComponentInit(ATTACK_TYPE_projectile_fire, id);
set->attack[index] = attack;
}
return success;
}

int DarkWizardSetAdd(DarkWizardSet *set, v3 spawn_position, SpriteComponent sprite)
{
int success = 0;
if(set->count < set->max_count)
{
success = 1;
u32 index = set->count++;
u32 id = DarkWizardSetGetNewID(set);
set->index_to_id_table[index] = id;
set->id_to_index_table[id] = index;
DarkWizardSetInitInstanceByIndex(set, index, spawn_position, sprite);
}
return success;
}

int DarkWizardSetAddDefault(DarkWizardSet *set, v3 spawn_position)
{
SpriteComponent sprite = SpriteComponentInit(spawn_position, 1.f, &app->art, v4(80, 176, 16, 16), 0, v4u(1));return DarkWizardSetAdd(set, spawn_position, sprite);
}

int DarkWizardSetRemoveByIndex(DarkWizardSet *set, u32 index)
{
int success = 0;

if(index >= 0 && index < set->count)
{
success = 1;
if(index != --set->count)
{
set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];
u32 replacement_index = set->count;
u32 replacement_id = set->index_to_id_table[replacement_index];
set->id_to_index_table[replacement_id] = replacement_index;
set->index_to_id_table[replacement_index] = replacement_id;
set->sphere[index] = set->sphere[replacement_index];
set->sprite[index] = set->sprite[replacement_index];
set->enemy_ai[index] = set->enemy_ai[replacement_index];
set->health[index] = set->health[replacement_index];
set->attack[index] = set->attack[replacement_index];
}

}

return success;
}

int DarkWizardSetRemoveByID(DarkWizardSet *set, u32 id)
{
return DarkWizardSetRemoveByIndex(set, set->id_to_index_table[id]);
}

void DarkWizardSetUpdate(DarkWizardSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)
{
UpdateSphereComponents(set->sphere, map, set->count);
TrackSpriteComponentsToSphereComponents(set->sprite, set->sphere, set->count, map);
UpdateSpriteComponents(set->sprite, set->count);
UpdateEnemyAI(set->enemy_ai, set->attack, set->count, player);
TrackEnemyAIComponentsToSphereComponents(set->enemy_ai, set->sphere, set->count);
TrackSphereComponentsToEnemyAIComponents(set->sphere, set->enemy_ai, set->count);
UpdateAttackComponents(set->attack, set->count, projectiles);
TrackAttackComponentsToSphereComponents(set->attack, set->sphere, set->count);
UpdateHealthComponents(set->health, set->count);
TrackHealthComponentsToSphereComponents(set->health, set->sphere, set->count);
TrackSpriteComponentsToHealthComponents(set->sprite, set->health, set->count);
for(u32 i = 0; i < set->count;)
{
    if(set->health[i].health <= 0.f)
{
        for(u32 j = 0; j < 100; ++j)
{
            ParticleSpawn(particles, PARTICLE_TYPE_strike, set->health[i].position, v3(RandomF32(-1, 1), RandomF32(-1, 1), RandomF32(-1, 1)));
        }
        DarkWizardSetRemoveByIndex(set, i);
    }
    else
{
        ++i;
    }
}

}

void DarkWizardSetRender(DarkWizardSet *set)
{
RenderSpriteComponents(set->sprite, set->count);
RenderSphereComponentsDebug(set->sphere, set->count);
RenderAttackComponentsDebug(set->attack, set->count);
}

void KnightSetInit(KnightSet *set, u32 maximum, MemoryArena *arena)
{
set->count = 0;
set->max_count = maximum;
set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->free_id_count = maximum;
set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);

for(u32 i = 0; i < maximum; ++i)
{
set->free_id_list[i] = maximum - i - 1;
}

set->sphere = (SphereComponent *)MemoryArenaAllocate(arena, sizeof(set->sphere[0]) * maximum);
set->sprite = (SpriteComponent *)MemoryArenaAllocate(arena, sizeof(set->sprite[0]) * maximum);
set->enemy_ai = (EnemyAIComponent *)MemoryArenaAllocate(arena, sizeof(set->enemy_ai[0]) * maximum);
set->health = (HealthComponent *)MemoryArenaAllocate(arena, sizeof(set->health[0]) * maximum);
set->attack = (AttackComponent *)MemoryArenaAllocate(arena, sizeof(set->attack[0]) * maximum);
set->weapon = (WeaponComponent *)MemoryArenaAllocate(arena, sizeof(set->weapon[0]) * maximum);
}

void KnightSetCleanUp(KnightSet *set)
{
}

u32 KnightSetGetNewID(KnightSet *set)
{
u32 id = 0;
if(set->free_id_count > 0)
{
--set->free_id_count;
id = set->free_id_list[set->free_id_count];
}
return id;
}

u32 KnightSetDevelopmentMatchesMapFileEntity(char *name)
{
return CStringMatchCaseInsensitive(name, "Knight");
}

int KnightSetInitInstanceByIndex(KnightSet *set, u32 index, v3 spawn_position, SpriteComponent sprite)
{
int success = 0;
if(index < set->max_count)
{
success = 1;
EntityID id = { (u16)KNIGHT_TYPE_ID, (u16)index, };
SphereComponent sphere = SphereComponentInit(spawn_position, 1.f);
set->sphere[index] = sphere;
set->sprite[index] = sprite;
EnemyAIComponent enemy_ai = EnemyAIComponentInit(ENEMY_AI_TYPE_melee);
set->enemy_ai[index] = enemy_ai;
HealthComponent health = HealthComponentInit(spawn_position, 1.f, 1.f, id);
set->health[index] = health;
AttackComponent attack = AttackComponentInit(ATTACK_TYPE_melee, id);
set->attack[index] = attack;
WeaponComponent weapon = WeaponComponentInitFromItemType(global_item_type_data+ITEM_TYPE_rusty_steel_sword);
set->weapon[index] = weapon;
}
return success;
}

int KnightSetAdd(KnightSet *set, v3 spawn_position, SpriteComponent sprite)
{
int success = 0;
if(set->count < set->max_count)
{
success = 1;
u32 index = set->count++;
u32 id = KnightSetGetNewID(set);
set->index_to_id_table[index] = id;
set->id_to_index_table[id] = index;
KnightSetInitInstanceByIndex(set, index, spawn_position, sprite);
}
return success;
}

int KnightSetAddDefault(KnightSet *set, v3 spawn_position)
{
SpriteComponent sprite = SpriteComponentInit(spawn_position, 1.f, &app->art, v4(192, 224, 16, 32), 0, v4u(1));return KnightSetAdd(set, spawn_position, sprite);
}

int KnightSetRemoveByIndex(KnightSet *set, u32 index)
{
int success = 0;

if(index >= 0 && index < set->count)
{
success = 1;
if(index != --set->count)
{
set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];
u32 replacement_index = set->count;
u32 replacement_id = set->index_to_id_table[replacement_index];
set->id_to_index_table[replacement_id] = replacement_index;
set->index_to_id_table[replacement_index] = replacement_id;
set->sphere[index] = set->sphere[replacement_index];
set->sprite[index] = set->sprite[replacement_index];
set->enemy_ai[index] = set->enemy_ai[replacement_index];
set->health[index] = set->health[replacement_index];
set->attack[index] = set->attack[replacement_index];
set->weapon[index] = set->weapon[replacement_index];
}

}

return success;
}

int KnightSetRemoveByID(KnightSet *set, u32 id)
{
return KnightSetRemoveByIndex(set, set->id_to_index_table[id]);
}

void KnightSetUpdate(KnightSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)
{
UpdateSphereComponents(set->sphere, map, set->count);
TrackSpriteComponentsToSphereComponents(set->sprite, set->sphere, set->count, map);
TrackWeaponComponentsToSphereComponents(set->weapon, set->sphere, set->count);
UpdateSpriteComponents(set->sprite, set->count);
UpdateEnemyAI(set->enemy_ai, set->attack, set->count, player);
TrackEnemyAIComponentsToSphereComponents(set->enemy_ai, set->sphere, set->count);
TrackSphereComponentsToEnemyAIComponents(set->sphere, set->enemy_ai, set->count);
UpdateAttackComponents(set->attack, set->count, projectiles);
TrackAttackComponentsToSphereComponents(set->attack, set->sphere, set->count);
UpdateHealthComponents(set->health, set->count);
TrackHealthComponentsToSphereComponents(set->health, set->sphere, set->count);
TrackSpriteComponentsToHealthComponents(set->sprite, set->health, set->count);
for(u32 i = 0; i < set->count;)
{
    if(set->health[i].health <= 0.f)
{
        for(u32 j = 0; j < 100; ++j)
{
            ParticleSpawn(particles, PARTICLE_TYPE_strike, set->health[i].position, v3(RandomF32(-1, 1), RandomF32(-1, 1), RandomF32(-1, 1)));
        }
        KnightSetRemoveByIndex(set, i);
    }
    else
{
        ++i;
    }
}

}

void KnightSetRender(KnightSet *set)
{
RenderSpriteComponents(set->sprite, set->count);
RenderWeaponComponents(set->weapon, set->count);
RenderSphereComponentsDebug(set->sphere, set->count);
RenderAttackComponentsDebug(set->attack, set->count);
}

void CrateSetInit(CrateSet *set, u32 maximum, MemoryArena *arena)
{
set->count = 0;
set->max_count = maximum;
set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);
set->free_id_count = maximum;
set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);

for(u32 i = 0; i < maximum; ++i)
{
set->free_id_list[i] = maximum - i - 1;
}

set->item = (ItemType **)MemoryArenaAllocate(arena, sizeof(set->item[0]) * maximum);
set->sphere = (SphereComponent *)MemoryArenaAllocate(arena, sizeof(set->sphere[0]) * maximum);
set->sprite = (SpriteComponent *)MemoryArenaAllocate(arena, sizeof(set->sprite[0]) * maximum);
set->health = (HealthComponent *)MemoryArenaAllocate(arena, sizeof(set->health[0]) * maximum);
}

void CrateSetCleanUp(CrateSet *set)
{
}

u32 CrateSetGetNewID(CrateSet *set)
{
u32 id = 0;
if(set->free_id_count > 0)
{
--set->free_id_count;
id = set->free_id_list[set->free_id_count];
}
return id;
}

u32 CrateSetDevelopmentMatchesMapFileEntity(char *name)
{
return CStringMatchCaseInsensitive(name, "Crate");
}

int CrateSetInitInstanceByIndex(CrateSet *set, u32 index, v3 spawn_position, ItemType *item)
{
int success = 0;
if(index < set->max_count)
{
success = 1;
EntityID id = { (u16)CRATE_TYPE_ID, (u16)index, };
set->item[index] = item;
SphereComponent sphere = SphereComponentInit(spawn_position, 1.f);
set->sphere[index] = sphere;
SpriteComponent sprite = SpriteComponentInit(spawn_position, 1.f, &app->art,  v4(80, 112, 16, 16), 0, v4u(1));
set->sprite[index] = sprite;
HealthComponent health = HealthComponentInit(spawn_position, 1.f, 1.f, id);
set->health[index] = health;
}
return success;
}

int CrateSetAdd(CrateSet *set, v3 spawn_position, ItemType *item)
{
int success = 0;
if(set->count < set->max_count)
{
success = 1;
u32 index = set->count++;
u32 id = CrateSetGetNewID(set);
set->index_to_id_table[index] = id;
set->id_to_index_table[id] = index;
CrateSetInitInstanceByIndex(set, index, spawn_position, item);
}
return success;
}

int CrateSetAddDefault(CrateSet *set, v3 spawn_position)
{
ItemType *item = global_item_type_data+0;return CrateSetAdd(set, spawn_position, item);
}

int CrateSetRemoveByIndex(CrateSet *set, u32 index)
{
int success = 0;

if(index >= 0 && index < set->count)
{
success = 1;
if(index != --set->count)
{
set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];
u32 replacement_index = set->count;
u32 replacement_id = set->index_to_id_table[replacement_index];
set->id_to_index_table[replacement_id] = replacement_index;
set->index_to_id_table[replacement_index] = replacement_id;
set->item[index] = set->item[replacement_index];
set->sphere[index] = set->sphere[replacement_index];
set->sprite[index] = set->sprite[replacement_index];
set->health[index] = set->health[replacement_index];
}

}

return success;
}

int CrateSetRemoveByID(CrateSet *set, u32 id)
{
return CrateSetRemoveByIndex(set, set->id_to_index_table[id]);
}

void CrateSetUpdate(CrateSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)
{
UpdateSphereComponents(set->sphere, map, set->count);
TrackSpriteComponentsToSphereComponents(set->sprite, set->sphere, set->count, map);
UpdateSpriteComponents(set->sprite, set->count);
UpdateHealthComponents(set->health, set->count);
TrackHealthComponentsToSphereComponents(set->health, set->sphere, set->count);
TrackSpriteComponentsToHealthComponents(set->sprite, set->health, set->count);
for(u32 i = 0; i < set->count;)
{
    if(set->health[i].health <= 0.f)
{
        for(u32 j = 0; j < 100; ++j)
{
            ParticleSpawn(particles, PARTICLE_TYPE_strike, set->health[i].position, v3(RandomF32(-1, 1), RandomF32(-1, 1), RandomF32(-1, 1)));
        }
        CrateSetRemoveByIndex(set, i);
    }
    else
{
        ++i;
    }
}

}

void CrateSetRender(CrateSet *set)
{
RenderSpriteComponents(set->sprite, set->count);
RenderSphereComponentsDebug(set->sphere, set->count);
}

