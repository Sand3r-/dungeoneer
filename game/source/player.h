#define PLAYER_INVENTORY_MAX 4

typedef struct ProjectileSet ProjectileSet;

typedef enum PlayerState PlayerState;
enum PlayerState
{
    PLAYER_STATE_free,
    PLAYER_STATE_get_item,
    PLAYER_STATE_dead,
    PLAYER_STATE_MAX
};

typedef struct Player Player;
struct Player
{
    PlayerState state;
    SphereComponent sphere;
    SpriteComponent sprite;
    AttackComponent attack;
    WeaponComponent weapon;
    HealthComponent health;
    ItemType *weapon_item;
    ItemType *inventory[PLAYER_INVENTORY_MAX];
    ItemType *get_item_item;
    f32 get_item_wait_time;
    f32 attack_velocity_boost;
    f32 inventory_view_transition;
};

internal void
PlayerInit(Player *player, v3 spawn);

internal b32
PlayerGetItem(Player *player, ItemType *item);

internal void
PlayerUpdate(Player *player, Map *map, Camera *camera, v3 last_safe_player_position,
             ProjectileSet *projectiles, ParticleMaster *particle);

internal void
PlayerRender(Player *player, ParticleMaster *particles);