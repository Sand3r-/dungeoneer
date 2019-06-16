internal Chunk
ChunkInit(MemoryArena *arena, v2 chunk_offset, u32 width, u32 height,
          MapTile *tiles, f32 *heights, f32 *vertex_data, u32 num_vertex_floats)
{
    Chunk chunk = {0};
    chunk.active = 1;
    chunk.width = width;
    chunk.height = height;
    chunk.model = ModelInitFromData(vertex_data, num_vertex_floats,
                                    MODEL_FLAG_POSITION |
                                    MODEL_FLAG_UV |
                                    MODEL_FLAG_NORMAL);
    chunk.tiles = tiles;
    chunk.heights = heights;
    return chunk;
}

internal void
ChunkCleanUp(Chunk *chunk)
{
    ModelCleanUp(&chunk->model);
    chunk->active = 0;
}

internal Map
MapInitFromLoadedMap(MemoryArena *arena, EntitySets *entities, LoadedMap *loaded_map)
{
    Map map = {0};
    map.player_spawn = loaded_map->player_spawn;
    
    iv2 active_chunk_lower_bound = {
        ClampI32(map.player_spawn.x / CHUNK_WIDTH_IN_TILES - MAP_CHUNK_SIZE/2, 0, loaded_map->chunk_width),
        ClampI32(map.player_spawn.y / CHUNK_HEIGHT_IN_TILES - MAP_CHUNK_SIZE/2, 0, loaded_map->chunk_height),
    };
    
    iv2 active_chunk_upper_bound = {
        ClampI32(map.player_spawn.x / CHUNK_WIDTH_IN_TILES + MAP_CHUNK_SIZE/2+1, 0, loaded_map->chunk_width),
        ClampI32(map.player_spawn.y / CHUNK_HEIGHT_IN_TILES + MAP_CHUNK_SIZE/2+1, 0, loaded_map->chunk_height),
    };
    
    for(i32 i = active_chunk_lower_bound.x;
        i < active_chunk_upper_bound.x && i < (i32)loaded_map->chunk_width;
        ++i)
    {
        for(i32 j = active_chunk_lower_bound.y;
            j < active_chunk_upper_bound.y && j < (i32)loaded_map->chunk_height;
            ++j)
        
        {
            LoadedChunk *loaded_chunk = loaded_map->chunks + j*loaded_map->chunk_width + i;
            map.chunks[i - active_chunk_lower_bound.x][j - active_chunk_lower_bound.y] =
                ChunkInit(arena, v2((f32)i*CHUNK_WIDTH_IN_TILES, (f32)j*CHUNK_HEIGHT_IN_TILES), loaded_chunk->width, loaded_chunk->height, loaded_chunk->tiles,
                          loaded_chunk->heights, loaded_chunk->vertex_data, loaded_chunk->vertex_data_float_count);
            
            loaded_chunk->SpawnEntities(entities);
        }
    }
    
    map.chunk_offset = active_chunk_lower_bound;
    map.chunk_size = iv2((i32)loaded_map->chunk_width, (i32)loaded_map->chunk_width);
    return map;
}

internal void
MapCleanUp(Map *map)
{
    for(int i = 0; i < MAP_CHUNK_SIZE; ++i)
    {
        for(int j = 0; j < MAP_CHUNK_SIZE; ++j)
        {
            ChunkCleanUp(&map->chunks[i][j]);
        }
    }
}

#if 0
internal void
MapUpdateChunksOnPosition(Map *map, v3 position, LoadedMap *loaded_map)
{
    
    iv2 last_active_chunk_lower_bound = map->chunk_offset;
    iv2 last_active_chunk_upper_bound = {
        last_active_chunk_lower_bound.x + MAP_CHUNK_SIZE+1,
        last_active_chunk_lower_bound.y + MAP_CHUNK_SIZE+1,
    };
    
    iv2 active_chunk_lower_bound = {
        ClampI32((i32)(position.x / TILE_SIZE) / CHUNK_WIDTH_IN_TILES - MAP_CHUNK_SIZE/2, 0, loaded_map->chunk_width),
        ClampI32((i32)(position.z / TILE_SIZE) / CHUNK_HEIGHT_IN_TILES - MAP_CHUNK_SIZE/2, 0, loaded_map->chunk_height),
    };
    
    iv2 active_chunk_upper_bound = {
        ClampI32((i32)(position.x / TILE_SIZE) / CHUNK_WIDTH_IN_TILES + MAP_CHUNK_SIZE/2+1, 0, loaded_map->chunk_width),
        ClampI32((i32)(position.z / TILE_SIZE) / CHUNK_HEIGHT_IN_TILES + MAP_CHUNK_SIZE/2+1, 0, loaded_map->chunk_height),
    };
    
    Chunk *new_chunks = MemoryArenaAllocate(&app->frame_arena, sizeof(Chunk)*MAP_CHUNK_SIZE*MAP_CHUNK_SIZE);
    
    for(i32 i = active_chunk_lower_bound.x;
        i < active_chunk_upper_bound.x && i < map->chunk_size.x;
        ++i)
    {
        for(i32 j = active_chunk_lower_bound.y;
            j < active_chunk_upper_bound.y && j < map->chunk_size.y;
            ++j)
        
        {
            LoadedChunk *loaded_chunk = loaded_map->chunks + j*loaded_map->chunk_width + i;
            if(i >= last_active_chunk_lower_bound.x && i < last_active_chunk_upper_bound.x &&
               j >= last_active_chunk_lower_bound.y && j < last_active_chunk_upper_bound.y)
            {
                new_chunks[j*MAP_CHUNK_SIZE + i] = map->chunks[i][j];
            }
            else
            {
                new_chunks[j*MAP_CHUNK_SIZE + i] =
                    ChunkInit(arena, v2((f32)i*CHUNK_WIDTH_IN_TILES, (f32)j*CHUNK_HEIGHT_IN_TILES), loaded_chunk->width, loaded_chunk->height, loaded_chunk->tiles,
                              loaded_chunk->heights, loaded_chunk->vertex_data, loaded_chunk->vertex_data_float_count);
            }
        }
    }
    
    map->chunk_offset = active_chunk_lower_bound;
}
#endif

internal f32
MapGetHeight(Map *map, iv2 coord)
{
    f32 height;
    {
        iv2 chunk_coord = {
            (coord.x - map->chunk_offset.x*CHUNK_WIDTH_IN_TILES) / CHUNK_WIDTH_IN_TILES,
            (coord.y - map->chunk_offset.y*CHUNK_HEIGHT_IN_TILES) / CHUNK_HEIGHT_IN_TILES,
        };
        
        if(chunk_coord.x >= 0 && chunk_coord.y >= 0)
        {
            
            Chunk *chunk = &map->chunks[chunk_coord.x][chunk_coord.y];
            
            iv2 chunk_relative_coords = {
                (coord.x - map->chunk_offset.x*CHUNK_WIDTH_IN_TILES) - chunk_coord.x*CHUNK_WIDTH_IN_TILES,
                (coord.y - map->chunk_offset.y*CHUNK_HEIGHT_IN_TILES) - chunk_coord.y*CHUNK_HEIGHT_IN_TILES,
            };
            
            if(chunk_relative_coords.x < 0 ||
               chunk_relative_coords.y < 0 ||
               chunk_relative_coords.x > CHUNK_WIDTH_IN_TILES ||
               chunk_relative_coords.y > CHUNK_HEIGHT_IN_TILES)
            {
                height = 0.f;
            }
            else
            {
                height = chunk->heights[(CHUNK_WIDTH_IN_TILES+1)*chunk_relative_coords.y + chunk_relative_coords.x];
            }
            
        }
    }
    return height;
}

internal f32
MapSampleHeight(Map *map, v2 position)
{
    iv2 s00 = { (i32)position.x/2, (i32)position.y/2 };
    iv2 s01 = { s00.x, s00.y+1 };
    iv2 s10 = { s00.x+1, s00.y };
    iv2 s11 = { s00.x+1, s00.y+1 };
    
    f32 height_00 = MapGetHeight(map, s00);
    f32 height_01 = MapGetHeight(map, s01);
    f32 height_10 = MapGetHeight(map, s10);
    f32 height_11 = MapGetHeight(map, s11);
    
    f32 height_00_to_10 = (height_00 + (height_10 - height_00) * (position.x/TILE_SIZE - (f32)s00.x));
    f32 height_01_to_11 = (height_01 + (height_11 - height_01) * (position.x/TILE_SIZE - (f32)s00.x));
    f32 height = height_00_to_10 + (height_01_to_11 - height_00_to_10) * (position.y/TILE_SIZE - (f32)s00.y);
    return height;
}

internal MapTile
MapGetTile(Map *map, iv2 coord)
{
    MapTile tile = {0};
    {
        iv2 chunk_coord = {
            (coord.x - map->chunk_offset.x*CHUNK_WIDTH_IN_TILES) / CHUNK_WIDTH_IN_TILES,
            (coord.y - map->chunk_offset.y*CHUNK_HEIGHT_IN_TILES) / CHUNK_HEIGHT_IN_TILES,
        };
        
        if(chunk_coord.x >= 0 && chunk_coord.y >= 0)
        {
            Chunk *chunk = &map->chunks[chunk_coord.x][chunk_coord.y];
            
            iv2 chunk_relative_coords = {
                (coord.x - map->chunk_offset.x*CHUNK_WIDTH_IN_TILES) - chunk_coord.x*CHUNK_WIDTH_IN_TILES,
                (coord.y - map->chunk_offset.y*CHUNK_HEIGHT_IN_TILES) - chunk_coord.y*CHUNK_HEIGHT_IN_TILES,
            };
            
            if(chunk_relative_coords.x < 0 ||
               chunk_relative_coords.y < 0 ||
               chunk_relative_coords.x >= CHUNK_WIDTH_IN_TILES ||
               chunk_relative_coords.y >= CHUNK_HEIGHT_IN_TILES)
            {}
            else
            {
                tile = chunk->tiles[(CHUNK_WIDTH_IN_TILES)*chunk_relative_coords.y + chunk_relative_coords.x];
            }
        }
    }
    return tile;
}

internal void
MapRender(Map *map)
{
    for(int i = 0; i < MAP_CHUNK_SIZE; ++i)
    {
        for(int j = 0; j < MAP_CHUNK_SIZE; ++j)
        {
            Chunk *chunk = &map->chunks[i][j];
            if(chunk->active)
            {
                m4 model = M4InitD(1.f);
                RendererPushModel(&app->renderer, model, &chunk->model, &app->art);
            }
        }
    }
}