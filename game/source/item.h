typedef enum ItemTypeIndex ItemTypeIndex;
enum ItemTypeIndex
{
    ITEM_TYPE_small_health_potion,
    ITEM_TYPE_wooden_sword,
    ITEM_TYPE_rusty_steel_sword,
    ITEM_TYPE_dagger,
    ITEM_TYPE_bomb,
    ITEM_TYPE_MAX
};

#define ITEM_FLAG_weapon (1<<0)
#define ITEM_FLAG_consumable (1<<1)

typedef struct ItemType ItemType;
struct ItemType
{
    char *name;
    v4 source;
    i32 flags;
    f32 health;
    f32 strength;
    f32 range;
};

global ItemType global_item_type_data[ITEM_TYPE_MAX] = {
    
    {
        "Small Health Potion",
        { 112, 208, 16, 16 },
        ITEM_FLAG_consumable,
        0.2f,
    },
    
    {
        "Wooden Sword",
        { 144, 0,   16, 32 },
        ITEM_FLAG_weapon,
        0,
        0.2f,
        0.6f,
    },
    
    {
        "Rusty Steel Sword",
        { 160, 0,   16, 32 },
        ITEM_FLAG_weapon,
        0,
        0.4f,
        0.8f,
    },
    
    {
        "Dagger",
        { 128, 96,  8,  16 },
        ITEM_FLAG_weapon,
        0,
        0.1f,
        0.4f,
    },
    
    {
        "Bomb",
        { 240, 48,  16, 16 },
        0,
    },
    
};