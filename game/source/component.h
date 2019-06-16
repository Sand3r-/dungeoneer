internal void UpdateSphereComponents(SphereComponent *sphere, Map *map, u32 n);
internal void TrackSpriteComponentsToSphereComponents(SpriteComponent *sprite, SphereComponent *sphere, u32 n, Map *map);
internal void UpdateSpriteComponents(SpriteComponent *sprite, u32 n);
internal void RenderSpriteComponents(SpriteComponent *sprite, u32 n);
internal void AttackComponentAttack(AttackComponent *attack, v3 position, f32 max_radius, f32 attack_time);
internal void TrackEnemyAIComponentsToSphereComponents(EnemyAIComponent *enemy_ai, SphereComponent *sphere, u32 n);