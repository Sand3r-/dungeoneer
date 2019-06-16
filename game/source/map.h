
typedef struct LoadedChunk LoadedChunk;
struct LoadedChunk
{
    u32 width;
    u32 height;
    MapTile *tiles;
    f32 *heights;
    f32 *vertex_data;
    u32 vertex_data_float_count;
    void (*SpawnEntities)(EntitySets *entities);
};

typedef struct LoadedMap LoadedMap;
struct LoadedMap
{
    u32 chunk_width;
    u32 chunk_height;
    LoadedChunk *chunks;
    iv2 player_spawn;
};

typedef struct Chunk Chunk;
struct Chunk
{
    b32 active;
    u32 width;
    u32 height;
    f32 *heights;
    MapTile *tiles;
    Model model;
};

#define MAP_CHUNK_SIZE 4

typedef struct Map Map;
struct Map
{
    iv2 chunk_offset;
    Chunk chunks[MAP_CHUNK_SIZE][MAP_CHUNK_SIZE];
    iv2 player_spawn;
    iv2 chunk_size;
};

internal Map
MapInitFromLoadedMap(MemoryArena *arena, EntitySets *entities, LoadedMap *loaded_map);

internal void
MapCleanUp(Map *map);

internal void
MapUpdateChunksOnPosition(Map *map, v3 position, LoadedMap *loaded_map);

internal f32
MapGetHeight(Map *map, iv2 coord);

internal f32
MapSampleHeight(Map *map, v2 position);

internal MapTile
MapGetTile(Map *map, iv2 position);

internal void
MapRender(Map *map);