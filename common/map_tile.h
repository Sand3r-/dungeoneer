#define MAP_TILE_WALL               (1<<0)
#define MAP_TILE_PIT                (1<<1)
#define MAP_TILE_DESTRUCTIBLE_WALL  (1<<2)

typedef struct MapTile MapTile;
struct MapTile
{
    u8 tx;
    u8 ty;
    u32 flags;
};
