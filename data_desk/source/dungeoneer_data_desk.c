#include <stdarg.h>
#include "data_desk.h"
#include "language_layer.h"
#include "constants.h"
#include "map_tile.h"

typedef struct MapNormal
{
    v3 normal;
    f32 count;
}
MapNormal;

typedef struct MemoryArena
{
    void *memory;
    u32 memory_size;
    u32 memory_alloc_pos;
    u32 memory_left;
}
MemoryArena;

internal MemoryArena
MemoryArenaInit(void *memory, u32 memory_size)
{
    MemoryArena m = {0};
    m.memory = memory;
    m.memory_size = memory_size;
    m.memory_left = memory_size;
    return m;
}

internal void *
MemoryArenaAllocate(MemoryArena *arena, u32 size)
{
    void *mem = 0;
    
    if(arena->memory_alloc_pos + size <= arena->memory_size)
    {
        mem = (void *)((u8 *)arena->memory + arena->memory_alloc_pos);
        arena->memory_alloc_pos += size;
        arena->memory_left -= size;
    }
    
    return mem;
}

internal void
MemoryArenaFreeBytes(MemoryArena *arena, u32 size)
{
    if(size > arena->memory_alloc_pos)
    {
        arena->memory_alloc_pos = 0;
        arena->memory_left = arena->memory_size;
    }
    else
    {
        arena->memory_alloc_pos -= size;
        arena->memory_left += size;
    }
}

internal void
MemoryArenaClear(MemoryArena *arena)
{
    arena->memory_alloc_pos = 0;
    arena->memory_left = arena->memory_size;
}

internal void
MemoryArenaZero(MemoryArena *arena)
{
    MemorySet(arena->memory, 0, arena->memory_size);
}

internal char *
MemoryArenaAllocateCStringCopy(MemoryArena *arena, char *str)
{
    u32 str_length = CalculateCStringLength(str);
    char *str_copy = (char *)MemoryArenaAllocate(arena, str_length+1);
    MemoryCopy(str_copy, str, str_length);
    str_copy[str_length] = 0;
    return str_copy;
}

static FILE *global_struct_header_file;
static FILE *global_struct_implementation_file;
static FILE *global_entity_header_file;
static FILE *global_entity_implementation_file;
static FILE *global_shader_header_file;
static FILE *global_shader_implementation_file;
static FILE *global_map_header_file;
static FILE *global_map_implementation_file;
static MemoryArena global_memory_arena;

static int
GenerateUICallForNode(FILE *file, DataDeskASTNode *node, char *target_format, ...);

typedef struct Entity
{
    char *name;
    char *name_lowercase_with_underscores;
    char *name_uppercase_with_underscores;
    char *name_upper_camel_case;
    char *name_lower_camel_case;
    char *name_with_spaces;
    DataDeskASTNode *root;
    int max_count;
    int entity_type_hash;
    int expected_size;
}
Entity;
static int global_entity_count = 0;
static Entity global_entities[4096];

typedef struct Component
{
    char *name;
    char *name_lowercase_with_underscores;
    char *name_uppercase_with_underscores;
    char *name_upper_camel_case;
    char *name_lower_camel_case;
    char *name_with_spaces;
    DataDeskASTNode *root;
}
Component;
static int global_component_count = 0;
static Component global_components[4096];

typedef struct Shader
{
    char name[64];
    char *filename;
    DataDeskASTNode *info;
    DataDeskASTNode *vert;
    DataDeskASTNode *frag;
}
Shader;
static int global_shader_count = 0;
static Shader global_shaders[4096];
static Shader *global_current_shader = 0;
static char *global_current_shader_filename = 0;

DATA_DESK_FUNC void
DataDeskCustomInitCallback(void)
{
    global_struct_header_file = fopen("generated/generated_structs.h", "w");
    global_entity_header_file = fopen("generated/generated_entity.h", "w");
    global_shader_header_file = fopen("generated/generated_shaders.h", "w");
    global_map_header_file    = fopen("generated/generated_maps.h", "w");
    
    static char file_buffer[Megabytes(64)] = {0};
    setvbuf(global_map_header_file, file_buffer, _IOFBF, sizeof(file_buffer));
    
    global_struct_implementation_file = fopen("generated/generated_structs.c", "w");
    global_entity_implementation_file = fopen("generated/generated_entity.c", "w");
    global_shader_implementation_file = fopen("generated/generated_shaders.c", "w");
    global_map_implementation_file    = fopen("generated/generated_maps.c", "w");
    
    static char arena_memory[Megabytes(32)] = {0};
    global_memory_arena = MemoryArenaInit(arena_memory, sizeof(arena_memory));
}

DATA_DESK_FUNC void
DataDeskCustomFileCallback(DataDeskASTNode *root, char *filename)
{}

DATA_DESK_FUNC void
DataDeskCustomConstantCallback(DataDeskConstant constant_info, char *filename)
{
    if(CStringContains(filename, "gl_shader_") &&
       CStringMatchCaseInsensitive(constant_info.name, "shader_name"))
    {
        if(global_current_shader_filename != filename || !global_current_shader)
        {
            global_current_shader = global_shaders + global_shader_count++;
            global_current_shader_filename = filename;
        }
        char *name = constant_info.root->constant_definition.expression->string + 1;
        int name_length = 0;
        for(; name[name_length] != '"' && name[name_length]; ++name_length);
        snprintf(global_current_shader->name, sizeof(global_current_shader->name),
                 "%.*s", name_length, name);
        global_current_shader->filename = filename;
        global_current_shader->info = constant_info.root;
    }
    else
    {
        FILE *file = global_struct_header_file;
        fprintf(file, "#define %s ", constant_info.name);
        DataDeskFWriteASTFromRootAsC(file, constant_info.root->constant_definition.expression, 1);
        fprintf(file, "\n");
    }
}

DATA_DESK_FUNC void
DataDeskCustomEnumCallback(DataDeskEnum enum_info, char *filename)
{
    FILE *file = global_struct_header_file;
    DataDeskFWriteASTFromRootAsC(file, enum_info.root, 0);
}

DATA_DESK_FUNC void
DataDeskCustomFlagsCallback(DataDeskFlags flags_info, char *filename)
{
    FILE *file = global_struct_header_file;
    DataDeskFWriteASTFromRootAsC(file, flags_info.root, 0);
}

DATA_DESK_FUNC void
DataDeskCustomStructCallback(DataDeskStruct struct_info, char *filename)
{
    if(DataDeskStructHasTag(struct_info, "Entity"))
    {
        if(global_entity_count < sizeof(global_entities) / sizeof(global_entities[0]))
        {
            int max_count = 256;
            
            char *max_count_num_string = DataDeskGetTagStringWithSubString(struct_info.root, "Entity(");
            if(max_count_num_string)
            {
                max_count = GetFirstI32FromCString(max_count_num_string);
            }
            
            int entity_type_hash = HashCString(struct_info.name_with_spaces) % (1 << 16);
            
            for(int i = 0; i < global_entity_count; ++i)
            {
                if(entity_type_hash == global_entities[i].entity_type_hash)
                {
                    fprintf(stderr, "WARNING: Entities \"%s\" and \"%s\" have a hash collision.\n",
                            struct_info.name_with_spaces, global_entities[i].name_with_spaces);
                }
            }
            
            Entity entity = {
                struct_info.name,
                struct_info.name_lowercase_with_underscores,
                struct_info.name_uppercase_with_underscores,
                struct_info.name_upper_camel_case,
                struct_info.name_lower_camel_case,
                struct_info.name_with_spaces,
                struct_info.root,
                max_count,
                entity_type_hash,
            };
            global_entities[global_entity_count++] = entity;
        }
    }
    else if(DataDeskStructHasTag(struct_info, "Component"))
    {
        if(global_component_count < sizeof(global_components) / sizeof(global_components[0]))
        {
            Component component = {
                struct_info.name,
                struct_info.name_lowercase_with_underscores,
                struct_info.name_uppercase_with_underscores,
                struct_info.name_upper_camel_case,
                struct_info.name_lower_camel_case,
                struct_info.name_with_spaces,
                struct_info.root,
            };
            global_components[global_component_count++] = component;
        }
        
        FILE *file = global_entity_header_file;
        
        DataDeskFWriteStructAsC(file, struct_info);
        
        fprintf(file, "%s %sInit(", struct_info.name, struct_info.name);
        
        int need_comma = 0;
        for(DataDeskASTNode *member = struct_info.root->struct_declaration.first_member;
            member;
            member = member->next)
        {
            if(!DataDeskNodeHasTag(member, "Zero"))
            {
                if(need_comma)
                {
                    fprintf(file, ", ");
                }
                
                if(DataDeskDeclarationIsType(member, "char") && member->declaration.type->type_usage.first_array_size_expression)
                {
                    fprintf(file, "char *%s", member->string);
                }
                else
                {
                    DataDeskFWriteASTFromRootAsC(file, member, 0);
                }
                
                need_comma = 1;
            }
        }
        fprintf(file, ")\n{\n");
        fprintf(file, "%s %s = {0};\n", struct_info.name, struct_info.name_lowercase_with_underscores);
        
        for(DataDeskASTNode *member = struct_info.root->struct_declaration.first_member;
            member;
            member = member->next)
        {
            if(!DataDeskNodeHasTag(member, "Zero"))
            {
                if(DataDeskDeclarationIsType(member, "char") && member->declaration.type->type_usage.first_array_size_expression)
                {
                    fprintf(file, "if(%s)\n{", member->string);
                    fprintf(file, "CopyCStringToFixedSizeBuffer(%s.%s, sizeof(%s.%s), %s);\n",
                            struct_info.name_lowercase_with_underscores, member->string,
                            struct_info.name_lowercase_with_underscores, member->string,
                            member->string);
                    fprintf(file, "}\n\n");
                }
                else
                {
                    fprintf(file, "%s.%s = %s;\n", struct_info.name_lowercase_with_underscores,
                            member->string, member->string);
                }
            }
        }
        
        fprintf(file, "return %s;\n", struct_info.name_lowercase_with_underscores);
        fprintf(file, "}\n\n");
    }
    else
    {
        DataDeskFWriteStructAsC(global_struct_header_file, struct_info);
        
        if(DataDeskStructHasTag(struct_info, "Printable"))
        {
            
            // NOTE(rjf): Generate function header
            {
                FILE *file = global_struct_header_file;
                fprintf(file, "static void %sPrint(%s *%s);\n", struct_info.name_upper_camel_case,
                        struct_info.name_upper_camel_case, struct_info.name_lowercase_with_underscores);
            }
            
            // NOTE(rjf): Generate function implementation
            {
                FILE *file = global_struct_implementation_file;
                fprintf(file, "static void\n%sPrint(%s *%s)\n{\n", struct_info.name_upper_camel_case,
                        struct_info.name_upper_camel_case, struct_info.name_lowercase_with_underscores);
                
                fprintf(file, "printf(\"{ \");\n");
                
                for(DataDeskASTNode *member = struct_info.root->struct_declaration.first_member;
                    member;
                    member = member->next)
                {
                    if(!DataDeskNodeHasTag(member, "NoPrint"))
                    {
                        if(DataDeskDeclarationIsType(member, "int") ||
                           DataDeskDeclarationIsType(member, "i8")  ||
                           DataDeskDeclarationIsType(member, "i16") ||
                           DataDeskDeclarationIsType(member, "i32") ||
                           DataDeskDeclarationIsType(member, "i64") ||
                           DataDeskDeclarationIsType(member, "u8")  ||
                           DataDeskDeclarationIsType(member, "u16") ||
                           DataDeskDeclarationIsType(member, "u32") ||
                           DataDeskDeclarationIsType(member, "u64"))
                        {
                            fprintf(file, "printf(\"%%i\", %s->%s);\n",
                                    struct_info.name_lowercase_with_underscores,
                                    member->string);
                        }
                        else if(DataDeskDeclarationIsType(member, "b8")  ||
                                DataDeskDeclarationIsType(member, "b16") ||
                                DataDeskDeclarationIsType(member, "b32") ||
                                DataDeskDeclarationIsType(member, "b64"))
                        {
                            fprintf(file, "printf(\"%%s\", %s->%s ? \"true\" : \"false\");\n",
                                    struct_info.name_lowercase_with_underscores,
                                    member->string);
                        }
                        else if(DataDeskDeclarationIsType(member, "*char"))
                        {
                            fprintf(file, "printf(\"\\\"%%s\\\"\", %s->%s);\n",
                                    struct_info.name_lowercase_with_underscores,
                                    member->string);
                        }
                        else if(DataDeskDeclarationIsType(member, "char"))
                        {
                            fprintf(file, "printf(\"%%c\", %s->%s);\n",
                                    struct_info.name_lowercase_with_underscores,
                                    member->string);
                        }
                        else if(DataDeskDeclarationIsType(member, "f32") ||
                                DataDeskDeclarationIsType(member, "f64") ||
                                DataDeskDeclarationIsType(member, "float") ||
                                DataDeskDeclarationIsType(member, "double"))
                        {
                            fprintf(file, "printf(\"%%f\", %s->%s);\n",
                                    struct_info.name_lowercase_with_underscores,
                                    member->string);
                        }
                        
                        fprintf(file, "printf(\", \");\n");
                    }
                }
                
                fprintf(file, "printf(\"}\");\n");
                
                fprintf(file, "}\n\n");
            }
        }
        
        if(DataDeskStructHasTag(struct_info, "UI"))
        {
            // NOTE(rjf): Generate function header
            {
                FILE *file = global_struct_header_file;
                fprintf(file, "static void %sUI(%s *%s);\n", struct_info.name_upper_camel_case,
                        struct_info.name_upper_camel_case, struct_info.name_lowercase_with_underscores);
            }
            
            // NOTE(rjf): Generate function implementation
            {
                FILE *file = global_struct_implementation_file;
                fprintf(file, "static void\n%sUI(%s *%s)\n{\n", struct_info.name_upper_camel_case,
                        struct_info.name_upper_camel_case, struct_info.name_lowercase_with_underscores);
                
                for(DataDeskASTNode *member = struct_info.root->struct_declaration.first_member;
                    member;
                    member = member->next)
                {
                    GenerateUICallForNode(file, member, "%s->%s", struct_info.name_lowercase_with_underscores,
                                          member->string);
                }
                
                fprintf(file, "}\n\n");
            }
        }
    }
}

static void
FWriteChunkData(FILE *file, char *name, int map_width, int map_height,
                MapTile *map_tiles, f32 *heights, MapNormal *normal_grid,
                int chunk_coord_x, int chunk_coord_y)
{
    int chunk_width = CHUNK_WIDTH_IN_TILES;
    int chunk_height = CHUNK_HEIGHT_IN_TILES;
    MemoryArena arena_backup = global_memory_arena;
    MemoryArena *arena = &global_memory_arena;
    
    fprintf(file, "global MapTile global_%s_chunk_%i%i_tiles[] = {\n",
            name, chunk_coord_x, chunk_coord_y);
    
    // NOTE(rjf): Generate tiles list
    {
        for(int y = chunk_coord_y*CHUNK_HEIGHT_IN_TILES;
            y < (chunk_coord_y+1)*CHUNK_HEIGHT_IN_TILES && y < map_height;
            ++y)
        {
            for(int x = chunk_coord_x*CHUNK_WIDTH_IN_TILES;
                x < (chunk_coord_x+1)*CHUNK_WIDTH_IN_TILES && x < map_width;
                ++x)
            {
                fprintf(file, "{%i,%i,%i},",
                        map_tiles[y*map_width+x].tx,
                        map_tiles[y*map_width+x].ty,
                        map_tiles[y*map_width+x].flags);
            }
        }
    }
    
    fprintf(file, "};\n\n");
    
    fprintf(file, "global f32 global_%s_chunk_%i%i_heights[] = {\n",
            name, chunk_coord_x, chunk_coord_y);
    
    for(u32 j = chunk_coord_y*chunk_height; j <= chunk_coord_y*chunk_height+chunk_height; ++j)
    {
        for(u32 i = chunk_coord_x*chunk_width; i <= chunk_coord_x*chunk_width+chunk_width; ++i)
        {
            fprintf(file, "%.6ff,", heights[j*(map_width+1) + i]);
            if(i % 20 == 0)
            {
                fprintf(file, "\n");
            }
        }
    }
    
    fprintf(file, "};\n\n");
    
    fprintf(file, "global f32 global_%s_chunk_%i%i_vertex_data[] = {\n",
            name, chunk_coord_x, chunk_coord_y);
    
    // NOTE(rjf): Generate map vertex data
    {
        
        // NOTE(rjf): Generate chunk geometry
        {
            u32 number_of_walls = 0;
            u32 number_of_pits = 0;
            f32 wall_height = 6.f;
            f32 pit_height = 20.f;
            
            // NOTE(rjf): Generate ground geometry
            {
                u32 floats_per_vertex = 8;
                u32 needed_floats = chunk_width * chunk_height * 6 * floats_per_vertex;
                f32 *vertices = MemoryArenaAllocate(arena, sizeof(f32)*needed_floats);
                f32 *vertex_data = vertices;
                
                for(u32 j = chunk_coord_y*chunk_height; j < chunk_coord_y*chunk_height+chunk_height; ++j)
                {
                    for(u32 i = chunk_coord_x*chunk_width; i < chunk_coord_x*chunk_width+chunk_width; ++i)
                    {
                        u32 index_00 = (map_width+1)*j + i;
                        u32 index_10 = (map_width+1)*j + i + 1;
                        u32 index_01 = (map_width+1)*(j+1) + i;
                        u32 index_11 = (map_width+1)*(j+1) + i + 1;
                        
                        u32 tile_index = map_width*j + i;
                        MapTile tile = map_tiles[tile_index];
                        
                        // NOTE(rjf): Determine if this tile requires wall borders.
                        if(tile.flags & MAP_TILE_WALL)
                        {
                            i32 tile_index_left  = (i > 0) ? (map_width*j + i - 1) : -1;
                            i32 tile_index_right = (i < map_width-1) ? (map_width*j + i + 1) : -1;
                            i32 tile_index_down  = (j < map_height-1) ? (map_width*(j+1) + i) : -1;
                            if(!(map_tiles[tile_index_left] .flags & MAP_TILE_WALL)) { number_of_walls += 1; }
                            if(!(map_tiles[tile_index_right].flags & MAP_TILE_WALL)) { number_of_walls += 1; }
                            if(!(map_tiles[tile_index_down] .flags & MAP_TILE_WALL)) { number_of_walls += 1; }
                        }
                        
                        // NOTE(rjf): Determine if this tile requires pit borders.
                        else if(tile.flags & MAP_TILE_PIT)
                        {
                            i32 tile_index_left  = (i > 0) ? (map_width*j + i - 1) : -1;
                            i32 tile_index_right = (i < map_width-1) ? (map_width*j + i + 1) : -1;
                            i32 tile_index_up    = (j < map_height-1) ? (map_width*(j-1) + i) : -1;
                            if(!(map_tiles[tile_index_left] .flags & MAP_TILE_PIT)) { number_of_pits += 1; }
                            if(!(map_tiles[tile_index_right].flags & MAP_TILE_PIT)) { number_of_pits += 1; }
                            if(!(map_tiles[tile_index_up]   .flags & MAP_TILE_PIT)) { number_of_pits += 1; }
                        }
                        
                        v3 tri_pos_1[3] = {
                            { (i)*TILE_SIZE,   heights[index_00], (j)*TILE_SIZE },
                            { (i)*TILE_SIZE,   heights[index_01], (j+1)*TILE_SIZE },
                            { (i+1)*TILE_SIZE, heights[index_10], (j)*TILE_SIZE },
                        };
                        
                        v3 tri_pos_2[3] = {
                            { (i+1)*TILE_SIZE, heights[index_10], (j)*TILE_SIZE },
                            { (i)*TILE_SIZE,   heights[index_01], (j+1)*TILE_SIZE },
                            { (i+1)*TILE_SIZE, heights[index_11], (j+1)*TILE_SIZE },
                        };
                        
                        if(tile.flags & MAP_TILE_WALL)
                        {
                            tri_pos_1[0].y += wall_height;
                            tri_pos_1[1].y += wall_height;
                            tri_pos_1[2].y += wall_height;
                            tri_pos_2[0].y += wall_height;
                            tri_pos_2[1].y += wall_height;
                            tri_pos_2[2].y += wall_height;
                        }
                        else if(tile.flags & MAP_TILE_PIT)
                        {
                            tri_pos_1[0].y -= 1000.f;
                            tri_pos_1[1].y -= 1000.f;
                            tri_pos_1[2].y -= 1000.f;
                            tri_pos_2[0].y -= 1000.f;
                            tri_pos_2[1].y -= 1000.f;
                            tri_pos_2[2].y -= 1000.f;
                        }
                        
                        v2 tri_uv_1[3] = {
                            { (16.f*(tile.tx+0)) / ART_WIDTH, (16.f*(tile.ty+0)) / ART_HEIGHT, },
                            { (16.f*(tile.tx+0)) / ART_WIDTH, (16.f*(tile.ty+1)) / ART_HEIGHT, },
                            { (16.f*(tile.tx+1)) / ART_WIDTH, (16.f*(tile.ty+0)) / ART_HEIGHT, },
                        };
                        
                        v2 tri_uv_2[3] = {
                            { (16.f*(tile.tx+1)) / ART_WIDTH, (16.f*(tile.ty+0)) / ART_HEIGHT, },
                            { (16.f*(tile.tx+0)) / ART_WIDTH, (16.f*(tile.ty+1)) / ART_HEIGHT, },
                            { (16.f*(tile.tx+1)) / ART_WIDTH, (16.f*(tile.ty+1)) / ART_HEIGHT, },
                        };
                        
                        vertex_data[0] = tri_pos_1[0].x;
                        vertex_data[1] = tri_pos_1[0].y;
                        vertex_data[2] = tri_pos_1[0].z;
                        vertex_data[3] = tri_uv_1[0].x;
                        vertex_data[4] = tri_uv_1[0].y;
                        vertex_data[5] = normal_grid[index_00].normal.x;
                        vertex_data[6] = normal_grid[index_00].normal.y;
                        vertex_data[7] = normal_grid[index_00].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[0] = tri_pos_1[1].x;
                        vertex_data[1] = tri_pos_1[1].y;
                        vertex_data[2] = tri_pos_1[1].z;
                        vertex_data[3] = tri_uv_1[1].x;
                        vertex_data[4] = tri_uv_1[1].y;
                        vertex_data[5] = normal_grid[index_01].normal.x;
                        vertex_data[6] = normal_grid[index_01].normal.y;
                        vertex_data[7] = normal_grid[index_01].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[0] = tri_pos_1[2].x;
                        vertex_data[1] = tri_pos_1[2].y;
                        vertex_data[2] = tri_pos_1[2].z;
                        vertex_data[3] = tri_uv_1[2].x;
                        vertex_data[4] = tri_uv_1[2].y;
                        vertex_data[5] = normal_grid[index_10].normal.x;
                        vertex_data[6] = normal_grid[index_10].normal.y;
                        vertex_data[7] = normal_grid[index_10].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[0] = tri_pos_2[0].x;
                        vertex_data[1] = tri_pos_2[0].y;
                        vertex_data[2] = tri_pos_2[0].z;
                        vertex_data[3] = tri_uv_2[0].x;
                        vertex_data[4] = tri_uv_2[0].y;
                        vertex_data[5] = normal_grid[index_10].normal.x;
                        vertex_data[6] = normal_grid[index_10].normal.y;
                        vertex_data[7] = normal_grid[index_10].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[0] = tri_pos_2[1].x;
                        vertex_data[1] = tri_pos_2[1].y;
                        vertex_data[2] = tri_pos_2[1].z;
                        vertex_data[3] = tri_uv_2[1].x;
                        vertex_data[4] = tri_uv_2[1].y;
                        vertex_data[5] = normal_grid[index_01].normal.x;
                        vertex_data[6] = normal_grid[index_01].normal.y;
                        vertex_data[7] = normal_grid[index_01].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[0] = tri_pos_2[2].x;
                        vertex_data[1] = tri_pos_2[2].y;
                        vertex_data[2] = tri_pos_2[2].z;
                        vertex_data[3] = tri_uv_2[2].x;
                        vertex_data[4] = tri_uv_2[2].y;
                        vertex_data[5] = normal_grid[index_11].normal.x;
                        vertex_data[6] = normal_grid[index_11].normal.y;
                        vertex_data[7] = normal_grid[index_11].normal.z;
                        vertex_data += floats_per_vertex;
                        
                    }
                }
                
                vertex_data = vertices;
                
                for(u32 j = chunk_coord_y*chunk_height; j < chunk_coord_y*chunk_height+chunk_height; ++j)
                {
                    for(u32 i = chunk_coord_x*chunk_width; i < chunk_coord_x*chunk_width+chunk_width; ++i)
                    {
                        u32 index_00 = (map_width+1)*j + i;
                        u32 index_10 = (map_width+1)*j + i + 1;
                        u32 index_01 = (map_width+1)*(j+1) + i;
                        u32 index_11 = (map_width+1)*(j+1) + i + 1;
                        
                        vertex_data[5] = normal_grid[index_00].normal.x;
                        vertex_data[6] = normal_grid[index_00].normal.y;
                        vertex_data[7] = normal_grid[index_00].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[5] = normal_grid[index_01].normal.x;
                        vertex_data[6] = normal_grid[index_01].normal.y;
                        vertex_data[7] = normal_grid[index_01].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[5] = normal_grid[index_10].normal.x;
                        vertex_data[6] = normal_grid[index_10].normal.y;
                        vertex_data[7] = normal_grid[index_10].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[5] = normal_grid[index_10].normal.x;
                        vertex_data[6] = normal_grid[index_10].normal.y;
                        vertex_data[7] = normal_grid[index_10].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[5] = normal_grid[index_01].normal.x;
                        vertex_data[6] = normal_grid[index_01].normal.y;
                        vertex_data[7] = normal_grid[index_01].normal.z;
                        vertex_data += floats_per_vertex;
                        
                        vertex_data[5] = normal_grid[index_11].normal.x;
                        vertex_data[6] = normal_grid[index_11].normal.y;
                        vertex_data[7] = normal_grid[index_11].normal.z;
                        vertex_data += floats_per_vertex;
                        
                    }
                }
                
                for(u32 i = 0; i < needed_floats;)
                {
                    fprintf(file, "%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,\n",
                            vertices[i], vertices[i+1], vertices[i+2], vertices[i+3],
                            vertices[i+4], vertices[i+5], vertices[i+6], vertices[i+7]);
                    i += floats_per_vertex;
                }
            }
            
            // NOTE(rjf): Generate wall geometry
            {
                u32 floats_per_vertex = 8;
                u32 vertices_per_tile = 6;
                u32 tiles_per_wall = (u32)(wall_height / TILE_SIZE);
                u32 tiles_per_pit = (u32)(pit_height / TILE_SIZE);
                u32 floats_per_wall = floats_per_vertex * vertices_per_tile * tiles_per_wall;
                u32 floats_per_pit = floats_per_vertex * vertices_per_tile * tiles_per_pit;
                u32 needed_floats = number_of_walls * floats_per_wall + number_of_pits * floats_per_pit;
                f32 *vertices = MemoryArenaAllocate(arena, sizeof(f32)*needed_floats);
                f32 *vertex_data = vertices;
                
                for(u32 j = chunk_coord_y*chunk_height; j < chunk_coord_y*chunk_height+chunk_height; ++j)
                {
                    for(u32 i = chunk_coord_x*chunk_width; i < chunk_coord_x*chunk_width+chunk_width; ++i)
                    {
                        u32 index_00 = (map_width+1)*j + i;
                        u32 index_10 = (map_width+1)*j + i + 1;
                        u32 index_01 = (map_width+1)*(j+1) + i;
                        u32 index_11 = (map_width+1)*(j+1) + i + 1;
                        
                        u32 tile_index = map_width*j + i;
                        MapTile tile = map_tiles[tile_index];
                        
                        if(tile.flags & MAP_TILE_WALL)
                        {
                            i32 tile_index_left  = (i > 0) ? (map_width*j + i - 1) : -1;
                            i32 tile_index_right = (i < map_width-1) ? (map_width*j + i + 1) : -1;
                            i32 tile_index_down  = (j < map_height-1) ? (map_width*(j+1) + i) : -1;
                            
                            b32 wall_left  = map_tiles[tile_index_left] .flags & MAP_TILE_WALL;
                            b32 wall_right = map_tiles[tile_index_right].flags & MAP_TILE_WALL;
                            b32 wall_down  = map_tiles[tile_index_down] .flags & MAP_TILE_WALL;
                            
                            for(u32 k = 0; k < tiles_per_wall; ++k)
                            {
                                if(!wall_left)
                                {
                                    
                                    v3 tri_pos_1[3] = {
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00],      (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00]+TILE_SIZE,  (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    v3 tri_pos_2[3] = {
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01],      (j+1)*TILE_SIZE  },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00],      (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    u8 tx = 1;
                                    u8 ty = 1;
                                    
                                    v2 tri_uv_1[3] = {
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v2 tri_uv_2[3] = {
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v3 tri_1_norm = CalculateTriangleNormalNormalized(tri_pos_1[0], tri_pos_1[1], tri_pos_1[2]);
                                    v3 tri_2_norm = CalculateTriangleNormalNormalized(tri_pos_2[0], tri_pos_2[1], tri_pos_2[2]);
                                    
                                    vertex_data[0] = tri_pos_1[0].x;
                                    vertex_data[1] = tri_pos_1[0].y;
                                    vertex_data[2] = tri_pos_1[0].z;
                                    vertex_data[3] = tri_uv_1[0].x;
                                    vertex_data[4] = tri_uv_1[0].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[1].x;
                                    vertex_data[1] = tri_pos_1[1].y;
                                    vertex_data[2] = tri_pos_1[1].z;
                                    vertex_data[3] = tri_uv_1[1].x;
                                    vertex_data[4] = tri_uv_1[1].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[2].x;
                                    vertex_data[1] = tri_pos_1[2].y;
                                    vertex_data[2] = tri_pos_1[2].z;
                                    vertex_data[3] = tri_uv_1[2].x;
                                    vertex_data[4] = tri_uv_1[2].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[0].x;
                                    vertex_data[1] = tri_pos_2[0].y;
                                    vertex_data[2] = tri_pos_2[0].z;
                                    vertex_data[3] = tri_uv_2[0].x;
                                    vertex_data[4] = tri_uv_2[0].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[1].x;
                                    vertex_data[1] = tri_pos_2[1].y;
                                    vertex_data[2] = tri_pos_2[1].z;
                                    vertex_data[3] = tri_uv_2[1].x;
                                    vertex_data[4] = tri_uv_2[1].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[2].x;
                                    vertex_data[1] = tri_pos_2[2].y;
                                    vertex_data[2] = tri_pos_2[2].z;
                                    vertex_data[3] = tri_uv_2[2].x;
                                    vertex_data[4] = tri_uv_2[2].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                }
                                
                                if(!wall_right)
                                {
                                    
                                    v3 tri_pos_1[3] = {
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_10],      (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_10]+TILE_SIZE,  (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_11]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    v3 tri_pos_2[3] = {
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_11],      (j+1)*TILE_SIZE  },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_10],      (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_11]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    u8 tx = 1;
                                    u8 ty = 1;
                                    
                                    v2 tri_uv_1[3] = {
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v2 tri_uv_2[3] = {
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v3 tri_1_norm = CalculateTriangleNormalNormalized(tri_pos_1[0], tri_pos_1[1], tri_pos_1[2]);
                                    v3 tri_2_norm = CalculateTriangleNormalNormalized(tri_pos_2[0], tri_pos_2[1], tri_pos_2[2]);
                                    
                                    tri_1_norm.x *= -1;
                                    tri_1_norm.y *= -1;
                                    tri_1_norm.z *= -1;
                                    tri_2_norm.x *= -1;
                                    tri_2_norm.y *= -1;
                                    tri_2_norm.z *= -1;
                                    
                                    vertex_data[0] = tri_pos_1[0].x;
                                    vertex_data[1] = tri_pos_1[0].y;
                                    vertex_data[2] = tri_pos_1[0].z;
                                    vertex_data[3] = tri_uv_1[0].x;
                                    vertex_data[4] = tri_uv_1[0].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[1].x;
                                    vertex_data[1] = tri_pos_1[1].y;
                                    vertex_data[2] = tri_pos_1[1].z;
                                    vertex_data[3] = tri_uv_1[1].x;
                                    vertex_data[4] = tri_uv_1[1].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[2].x;
                                    vertex_data[1] = tri_pos_1[2].y;
                                    vertex_data[2] = tri_pos_1[2].z;
                                    vertex_data[3] = tri_uv_1[2].x;
                                    vertex_data[4] = tri_uv_1[2].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[0].x;
                                    vertex_data[1] = tri_pos_2[0].y;
                                    vertex_data[2] = tri_pos_2[0].z;
                                    vertex_data[3] = tri_uv_2[0].x;
                                    vertex_data[4] = tri_uv_2[0].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[1].x;
                                    vertex_data[1] = tri_pos_2[1].y;
                                    vertex_data[2] = tri_pos_2[1].z;
                                    vertex_data[3] = tri_uv_2[1].x;
                                    vertex_data[4] = tri_uv_2[1].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[2].x;
                                    vertex_data[1] = tri_pos_2[2].y;
                                    vertex_data[2] = tri_pos_2[2].z;
                                    vertex_data[3] = tri_uv_2[2].x;
                                    vertex_data[4] = tri_uv_2[2].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                }
                                
                                if(!wall_down)
                                {
                                    
                                    v3 tri_pos_1[3] = {
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01],      (j+1)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01]+TILE_SIZE,  (j+1)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE, k*TILE_SIZE+heights[index_11]+TILE_SIZE,  (j+1)*TILE_SIZE    },
                                    };
                                    
                                    v3 tri_pos_2[3] = {
                                        { (i+1)*TILE_SIZE, k*TILE_SIZE+heights[index_11],      (j+1)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01],      (j+1)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE, k*TILE_SIZE+heights[index_11]+TILE_SIZE,  (j+1)*TILE_SIZE    },
                                    };
                                    
                                    u8 tx = 1;
                                    u8 ty = 1;
                                    
                                    v2 tri_uv_1[3] = {
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v2 tri_uv_2[3] = {
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v3 tri_1_norm = CalculateTriangleNormalNormalized(tri_pos_1[0], tri_pos_1[1], tri_pos_1[2]);
                                    v3 tri_2_norm = CalculateTriangleNormalNormalized(tri_pos_2[0], tri_pos_2[1], tri_pos_2[2]);
                                    
                                    vertex_data[0] = tri_pos_1[0].x;
                                    vertex_data[1] = tri_pos_1[0].y;
                                    vertex_data[2] = tri_pos_1[0].z;
                                    vertex_data[3] = tri_uv_1[0].x;
                                    vertex_data[4] = tri_uv_1[0].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[1].x;
                                    vertex_data[1] = tri_pos_1[1].y;
                                    vertex_data[2] = tri_pos_1[1].z;
                                    vertex_data[3] = tri_uv_1[1].x;
                                    vertex_data[4] = tri_uv_1[1].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[2].x;
                                    vertex_data[1] = tri_pos_1[2].y;
                                    vertex_data[2] = tri_pos_1[2].z;
                                    vertex_data[3] = tri_uv_1[2].x;
                                    vertex_data[4] = tri_uv_1[2].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[0].x;
                                    vertex_data[1] = tri_pos_2[0].y;
                                    vertex_data[2] = tri_pos_2[0].z;
                                    vertex_data[3] = tri_uv_2[0].x;
                                    vertex_data[4] = tri_uv_2[0].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[1].x;
                                    vertex_data[1] = tri_pos_2[1].y;
                                    vertex_data[2] = tri_pos_2[1].z;
                                    vertex_data[3] = tri_uv_2[1].x;
                                    vertex_data[4] = tri_uv_2[1].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[2].x;
                                    vertex_data[1] = tri_pos_2[2].y;
                                    vertex_data[2] = tri_pos_2[2].z;
                                    vertex_data[3] = tri_uv_2[2].x;
                                    vertex_data[4] = tri_uv_2[2].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                }
                                
                            }
                        }
                        
                        else if(tile.flags & MAP_TILE_PIT)
                        {
                            i32 tile_index_left  = (i > 0) ? (map_width*j + i - 1) : -1;
                            i32 tile_index_right = (i < map_width-1) ? (map_width*j + i + 1) : -1;
                            i32 tile_index_up    = (j > 0) ? (map_width*(j-1) + i) : -1;
                            
                            b32 pit_left  = tile_index_left >= 0  ? map_tiles[tile_index_left] .flags & MAP_TILE_PIT : 1;
                            b32 pit_right = tile_index_right >= 0 ? map_tiles[tile_index_right].flags & MAP_TILE_PIT : 1;
                            b32 pit_up    = tile_index_up >= 0    ? map_tiles[tile_index_up]   .flags & MAP_TILE_PIT : 1;
                            
                            for(i32 k = -1; k >= -(i32)tiles_per_pit; --k)
                            {
                                if(!pit_left)
                                {
                                    
                                    v3 tri_pos_1[3] = {
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00],            (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00]+TILE_SIZE,  (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    v3 tri_pos_2[3] = {
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01],            (j+1)*TILE_SIZE  },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00],            (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_01]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    u8 tx = 1;
                                    u8 ty = 1;
                                    
                                    v2 tri_uv_1[3] = {
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v2 tri_uv_2[3] = {
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v3 tri_1_norm = CalculateTriangleNormalNormalized(tri_pos_1[0], tri_pos_1[1], tri_pos_1[2]);
                                    v3 tri_2_norm = CalculateTriangleNormalNormalized(tri_pos_2[0], tri_pos_2[1], tri_pos_2[2]);
                                    
                                    tri_1_norm.x *= -1;
                                    tri_1_norm.y *= -1;
                                    tri_1_norm.z *= -1;
                                    tri_2_norm.x *= -1;
                                    tri_2_norm.y *= -1;
                                    tri_2_norm.z *= -1;
                                    
                                    vertex_data[0] = tri_pos_1[0].x;
                                    vertex_data[1] = tri_pos_1[0].y;
                                    vertex_data[2] = tri_pos_1[0].z;
                                    vertex_data[3] = tri_uv_1[0].x;
                                    vertex_data[4] = tri_uv_1[0].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[1].x;
                                    vertex_data[1] = tri_pos_1[1].y;
                                    vertex_data[2] = tri_pos_1[1].z;
                                    vertex_data[3] = tri_uv_1[1].x;
                                    vertex_data[4] = tri_uv_1[1].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[2].x;
                                    vertex_data[1] = tri_pos_1[2].y;
                                    vertex_data[2] = tri_pos_1[2].z;
                                    vertex_data[3] = tri_uv_1[2].x;
                                    vertex_data[4] = tri_uv_1[2].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[0].x;
                                    vertex_data[1] = tri_pos_2[0].y;
                                    vertex_data[2] = tri_pos_2[0].z;
                                    vertex_data[3] = tri_uv_2[0].x;
                                    vertex_data[4] = tri_uv_2[0].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[1].x;
                                    vertex_data[1] = tri_pos_2[1].y;
                                    vertex_data[2] = tri_pos_2[1].z;
                                    vertex_data[3] = tri_uv_2[1].x;
                                    vertex_data[4] = tri_uv_2[1].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[2].x;
                                    vertex_data[1] = tri_pos_2[2].y;
                                    vertex_data[2] = tri_pos_2[2].z;
                                    vertex_data[3] = tri_uv_2[2].x;
                                    vertex_data[4] = tri_uv_2[2].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                }
                                
                                if(!pit_right)
                                {
                                    
                                    v3 tri_pos_1[3] = {
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_10],      (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_10]+TILE_SIZE,  (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_11]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    v3 tri_pos_2[3] = {
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_11],      (j+1)*TILE_SIZE  },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_10],      (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE,   k*TILE_SIZE+heights[index_11]+TILE_SIZE,  (j+1)*TILE_SIZE  },
                                    };
                                    
                                    u8 tx = 1;
                                    u8 ty = 1;
                                    
                                    v2 tri_uv_1[3] = {
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v2 tri_uv_2[3] = {
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v3 tri_1_norm = CalculateTriangleNormalNormalized(tri_pos_1[0], tri_pos_1[1], tri_pos_1[2]);
                                    v3 tri_2_norm = CalculateTriangleNormalNormalized(tri_pos_2[0], tri_pos_2[1], tri_pos_2[2]);
                                    
                                    vertex_data[0] = tri_pos_1[0].x;
                                    vertex_data[1] = tri_pos_1[0].y;
                                    vertex_data[2] = tri_pos_1[0].z;
                                    vertex_data[3] = tri_uv_1[0].x;
                                    vertex_data[4] = tri_uv_1[0].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[1].x;
                                    vertex_data[1] = tri_pos_1[1].y;
                                    vertex_data[2] = tri_pos_1[1].z;
                                    vertex_data[3] = tri_uv_1[1].x;
                                    vertex_data[4] = tri_uv_1[1].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[2].x;
                                    vertex_data[1] = tri_pos_1[2].y;
                                    vertex_data[2] = tri_pos_1[2].z;
                                    vertex_data[3] = tri_uv_1[2].x;
                                    vertex_data[4] = tri_uv_1[2].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[0].x;
                                    vertex_data[1] = tri_pos_2[0].y;
                                    vertex_data[2] = tri_pos_2[0].z;
                                    vertex_data[3] = tri_uv_2[0].x;
                                    vertex_data[4] = tri_uv_2[0].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[1].x;
                                    vertex_data[1] = tri_pos_2[1].y;
                                    vertex_data[2] = tri_pos_2[1].z;
                                    vertex_data[3] = tri_uv_2[1].x;
                                    vertex_data[4] = tri_uv_2[1].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[2].x;
                                    vertex_data[1] = tri_pos_2[2].y;
                                    vertex_data[2] = tri_pos_2[2].z;
                                    vertex_data[3] = tri_uv_2[2].x;
                                    vertex_data[4] = tri_uv_2[2].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                }
                                
                                if(!pit_up)
                                {
                                    
                                    v3 tri_pos_1[3] = {
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00],            (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00]+TILE_SIZE,  (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE, k*TILE_SIZE+heights[index_10]+TILE_SIZE,  (j)*TILE_SIZE    },
                                    };
                                    
                                    v3 tri_pos_2[3] = {
                                        { (i+1)*TILE_SIZE, k*TILE_SIZE+heights[index_10],            (j)*TILE_SIZE    },
                                        { (i)*TILE_SIZE,   k*TILE_SIZE+heights[index_00],            (j)*TILE_SIZE    },
                                        { (i+1)*TILE_SIZE, k*TILE_SIZE+heights[index_10]+TILE_SIZE,  (j)*TILE_SIZE    },
                                    };
                                    
                                    u8 tx = 1;
                                    u8 ty = 1;
                                    
                                    v2 tri_uv_1[3] = {
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v2 tri_uv_2[3] = {
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+0)) / ART_WIDTH, (16.f*(ty+1)) / ART_HEIGHT, },
                                        { (16.f*(tx+1)) / ART_WIDTH, (16.f*(ty+0)) / ART_HEIGHT, },
                                    };
                                    
                                    v3 tri_1_norm = CalculateTriangleNormalNormalized(tri_pos_1[0], tri_pos_1[1], tri_pos_1[2]);
                                    v3 tri_2_norm = CalculateTriangleNormalNormalized(tri_pos_2[0], tri_pos_2[1], tri_pos_2[2]);
                                    
                                    vertex_data[0] = tri_pos_1[0].x;
                                    vertex_data[1] = tri_pos_1[0].y;
                                    vertex_data[2] = tri_pos_1[0].z;
                                    vertex_data[3] = tri_uv_1[0].x;
                                    vertex_data[4] = tri_uv_1[0].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[1].x;
                                    vertex_data[1] = tri_pos_1[1].y;
                                    vertex_data[2] = tri_pos_1[1].z;
                                    vertex_data[3] = tri_uv_1[1].x;
                                    vertex_data[4] = tri_uv_1[1].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_1[2].x;
                                    vertex_data[1] = tri_pos_1[2].y;
                                    vertex_data[2] = tri_pos_1[2].z;
                                    vertex_data[3] = tri_uv_1[2].x;
                                    vertex_data[4] = tri_uv_1[2].y;
                                    vertex_data[5] = tri_1_norm.x;
                                    vertex_data[6] = tri_1_norm.y;
                                    vertex_data[7] = tri_1_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[0].x;
                                    vertex_data[1] = tri_pos_2[0].y;
                                    vertex_data[2] = tri_pos_2[0].z;
                                    vertex_data[3] = tri_uv_2[0].x;
                                    vertex_data[4] = tri_uv_2[0].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[1].x;
                                    vertex_data[1] = tri_pos_2[1].y;
                                    vertex_data[2] = tri_pos_2[1].z;
                                    vertex_data[3] = tri_uv_2[1].x;
                                    vertex_data[4] = tri_uv_2[1].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                    vertex_data[0] = tri_pos_2[2].x;
                                    vertex_data[1] = tri_pos_2[2].y;
                                    vertex_data[2] = tri_pos_2[2].z;
                                    vertex_data[3] = tri_uv_2[2].x;
                                    vertex_data[4] = tri_uv_2[2].y;
                                    vertex_data[5] = tri_2_norm.x;
                                    vertex_data[6] = tri_2_norm.y;
                                    vertex_data[7] = tri_2_norm.z;
                                    vertex_data += floats_per_vertex;
                                    
                                }
                                
                            }
                            
                        }
                        
                    }
                    
                }
                
                for(u32 i = 0; i < needed_floats;)
                {
                    fprintf(file, "%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,%.6ff,\n",
                            vertices[i], vertices[i+1], vertices[i+2], vertices[i+3],
                            vertices[i+4], vertices[i+5], vertices[i+6], vertices[i+7]);
                    i += 8;
                }
            }
            
        }
        
    }
    
    fprintf(file, "};\n\n");
    *arena = arena_backup;
}

static void
FWriteMapData(FILE *file, char *string, char *name)
{
    string += 4;
    
    static char map_tile_grid[4096][4096] = {0};
    MemorySet(map_tile_grid, 0, sizeof(map_tile_grid));
    static MapTile map_tiles[Megabytes(2) / sizeof(MapTile)] = {0};
    
    int player_spawn_x = 0;
    int player_spawn_y = 0;
    int map_width = 0;
    int map_height = 0;
    int max_map_width = 0;
    
    for(int i = 0; string[i]; ++i)
    {
        if(string[i] == '"' && string[i+1] == '"' && string[i+2] == '"')
        {
            ++map_height;
            if(map_width > max_map_width)
            {
                max_map_width = map_width;
            }
            break;
        }
        else if(string[i] == '\n')
        {
            ++map_height;
            if(map_width > max_map_width)
            {
                max_map_width = map_width;
            }
            map_width = 0;
        }
        else
        {
            map_tile_grid[map_height][map_width] = string[i];
            ++map_width;
        }
    }
    
    map_width = max_map_width;
    
    // NOTE(rjf): Fill out empty tiles
    int needed_map_width = map_width;
    needed_map_width += needed_map_width % CHUNK_WIDTH_IN_TILES;
    int needed_map_height = map_height;
    needed_map_height += needed_map_height % CHUNK_HEIGHT_IN_TILES;
    for(int y = map_height; y < needed_map_height; ++y)
    {
        for(int x = map_width; x < needed_map_width; ++x)
        {
            map_tile_grid[y][x] = '#';
        }
    }
    map_width = needed_map_width;
    map_height = needed_map_height;
    for(int y = 0; y < map_height; ++y)
    {
        for(int x = 0; x < map_width; ++x)
        {
            if(map_tile_grid[y][x] == 0)
            {
                map_tile_grid[y][x] = '#';
            }
        }
    }
    
    for(int y = 0; y < map_height; ++y)
    {
        for(int x = 0; x < map_width; ++x)
        {
            
            char tile;
            if(x >= 0 && y >= 0 && x < map_width && y < map_height)
            {
                tile = map_tile_grid[y][x];
            }
            else
            {
                tile = '#';
            }
            
            int tx = 0;
            int ty = 0;
            int flags = 0;
            
            switch(tile)
            {
                default:
                case '.':
                {
                    if(y == 0 || map_tile_grid[y-1][x] == '#')
                    {
                        tx = 0;
                        ty = 2;
                    }
                    else
                    {
                        tx = 2;
                        ty = 3;
                    }
                    
                    break;
                }
                
                case '_':
                {
                    int ground_up    = y > 0            && map_tile_grid[y-1][x] == '_';
                    int ground_down  = y < map_height-1 && map_tile_grid[y+1][x] == '_';
                    int ground_left  = x > 0            && map_tile_grid[y][x-1] == '_';
                    int ground_right = x < map_width-1  && map_tile_grid[y][x+1] == '_';
                    
                    if(ground_up && ground_down && ground_left && ground_right)
                    {
                        tx = 4;
                        ty = 7;
                    }
                    else
                    {
                        tx = 2;
                        ty = 7;
                        
                        if(!ground_left)
                        {
                            tx = 0;
                        }
                        else if(!ground_right)
                        {
                            tx = 3;
                        }
                        
                        if(!ground_up)
                        {
                            ty = 6;
                        }
                        else if(!ground_down)
                        {
                            ty = 8;
                        }
                    }
                    
                    break;
                }
                
                case 'O':
                {
                    flags = MAP_TILE_PIT;
                    break;
                }
                
                case '#':
                {
                    tx = 4;
                    ty = 6;
                    flags = MAP_TILE_WALL;
                    
                    int wall_up    = y > 0            && map_tile_grid[y-1][x] == '#';
                    int wall_down  = y < map_height-1 && map_tile_grid[y+1][x] == '#';
                    int wall_left  = x > 0            && map_tile_grid[y][x-1] == '#';
                    int wall_right = x < map_width-1  && map_tile_grid[y][x+1] == '#';
                    
                    if(wall_up && wall_down && wall_left && wall_right)
                    {
                        tx = 4;
                        ty = 6;
                    }
                    else
                    {
                        if(!wall_left && !wall_down)
                        {
                            tx = 4;
                            ty = 15;
                        }
                        else if(!wall_right && !wall_down)
                        {
                            tx = 4;
                            ty = 14;
                        }
                        else if(!wall_left && !wall_up)
                        {
                            tx = 5;
                            ty = 15;
                        }
                        else if(!wall_right && !wall_up)
                        {
                            tx = 5;
                            ty = 14;
                        }
                        else if(!wall_up)
                        {
                            tx = 2;
                            ty = 15;
                        }
                        else if(!wall_down)
                        {
                            tx = 0;
                            ty = 15;
                        }
                        else if(!wall_left)
                        {
                            tx = 1;
                            ty = 15;
                        }
                        else if(!wall_right)
                        {
                            tx = 3;
                            ty = 15;
                        }
                    }
                    
                    break;
                }
            }
            
            map_tiles[y*map_width + x].tx = tx;
            map_tiles[y*map_width + x].ty = ty;
            map_tiles[y*map_width + x].flags = flags;
        }
    }
    
    for(int y = 0; y < map_height; ++y)
    {
        for(int x = 0; x < map_width; ++x)
        {
            if(map_tile_grid[y][x] == 'P')
            {
                player_spawn_x = x;
                player_spawn_y = y;
            }
        }
    }
    
    // NOTE(rjf): Generate heights
    f32 *heights = MemoryArenaAllocate(&global_memory_arena, sizeof(f32)*(map_width+1)*(map_height+1));
    {
        for(u32 j = 0; j <= map_height; ++j)
        {
            for(u32 i = 0; i <= map_width; ++i)
            {
                heights[j*(map_width+1) + i] = Perlin2D((f32)i, (f32)j, 0.2f, 4) * 3.5f;
            }
        }
    }
    
    // NOTE(rjf): Generate normals
    MapNormal *normal_grid = MemoryArenaAllocate(&global_memory_arena, sizeof(MapNormal)*(map_width+1)*(map_height+1));
    {
        MemorySet(normal_grid, 0, sizeof(MapNormal)*(map_width+1)*(map_height+1));
        for(u32 j = 0; j <= map_height; ++j)
        {
            for(u32 i = 0; i <= map_width; ++i)
            {
                u32 index_00 = (map_width+1)*j + i;
                u32 index_10 = (map_width+1)*j + i + 1;
                u32 index_01 = (map_width+1)*(j+1) + i;
                u32 index_11 = (map_width+1)*(j+1) + i + 1;
                
                v3 tri_pos_1[3] = {
                    { (i)*TILE_SIZE,   heights[index_00], (j)*TILE_SIZE },
                    { (i)*TILE_SIZE,   heights[index_01], (j+1)*TILE_SIZE },
                    { (i+1)*TILE_SIZE, heights[index_10], (j)*TILE_SIZE },
                };
                
                v3 tri_pos_2[3] = {
                    { (i+1)*TILE_SIZE, heights[index_10], (j)*TILE_SIZE },
                    { (i)*TILE_SIZE,   heights[index_01], (j+1)*TILE_SIZE },
                    { (i+1)*TILE_SIZE, heights[index_11], (j+1)*TILE_SIZE },
                };
                
                v3 tri_1_norm = CalculateTriangleNormal(tri_pos_1[0], tri_pos_1[1], tri_pos_1[2]);
                v3 tri_2_norm = CalculateTriangleNormal(tri_pos_2[0], tri_pos_2[1], tri_pos_2[2]);
                
                normal_grid[index_00].normal.x += tri_1_norm.x;
                normal_grid[index_00].normal.y += tri_1_norm.y;
                normal_grid[index_00].normal.z += tri_1_norm.z;
                normal_grid[index_00].count += 1;
                
                normal_grid[index_01].normal.x += tri_1_norm.x;
                normal_grid[index_01].normal.y += tri_1_norm.y;
                normal_grid[index_01].normal.z += tri_1_norm.z;
                normal_grid[index_01].normal.x += tri_2_norm.x;
                normal_grid[index_01].normal.y += tri_2_norm.y;
                normal_grid[index_01].normal.z += tri_2_norm.z;
                normal_grid[index_01].count += 2;
                
                normal_grid[index_10].normal.x += tri_1_norm.x;
                normal_grid[index_10].normal.y += tri_1_norm.y;
                normal_grid[index_10].normal.z += tri_1_norm.z;
                normal_grid[index_10].normal.x += tri_2_norm.x;
                normal_grid[index_10].normal.y += tri_2_norm.y;
                normal_grid[index_10].normal.z += tri_2_norm.z;
                normal_grid[index_10].count += 2;
                
                normal_grid[index_11].normal.x += tri_2_norm.x;
                normal_grid[index_11].normal.y += tri_2_norm.y;
                normal_grid[index_11].normal.z += tri_2_norm.z;
                normal_grid[index_11].count += 1;
                
            }
        }
        
        for(u32 j = 0; j <= map_height; ++j)
        {
            for(u32 i = 0; i <= map_width; ++i)
            {
                u32 index = j*(map_width+1)+i;
                normal_grid[index].normal.x /= (f32)normal_grid[index].count;
                normal_grid[index].normal.y /= (f32)normal_grid[index].count;
                normal_grid[index].normal.z /= (f32)normal_grid[index].count;
                normal_grid[index].normal = V3Normalize(normal_grid[index].normal);
            }
        }
        
    }
    
    int max_chunks_x = map_width / CHUNK_WIDTH_IN_TILES + 1;
    int max_chunks_y = map_height / CHUNK_HEIGHT_IN_TILES + 1;
    
    for(int chunk_y = 0; chunk_y < max_chunks_y; ++chunk_y)
    {
        for(int chunk_x = 0; chunk_x < max_chunks_x; ++chunk_x)
        {
            FWriteChunkData(file, name, map_width, map_height, map_tiles, heights, normal_grid, chunk_x, chunk_y);
            
            
            fprintf(file, "internal void SpawnEntitiesForChunk_%s_%i%i(EntitySets *entities)\n{\n",
                    name, chunk_x, chunk_y);
            {
                for(int y = chunk_y*CHUNK_HEIGHT_IN_TILES; y < (chunk_y+1)*CHUNK_HEIGHT_IN_TILES; ++y)
                {
                    for(int x = chunk_x*CHUNK_WIDTH_IN_TILES; x < (chunk_x+1)*CHUNK_WIDTH_IN_TILES; ++x)
                    {
                        char tile = map_tile_grid[y][x];
                        switch(tile)
                        {
                            
                            case '*':
                            {
                                fprintf(file, "TorchSetAdd(&entities->torch_set, v3(%f, 0.f, %f));\n",
                                        x*TILE_SIZE + TILE_SIZE/2, y*TILE_SIZE + TILE_SIZE/2);
                                break;
                            }
                            
                            case '|':
                            {
                                fprintf(file, "TreasureChestSetAdd(&entities->treasure_chest_set, v3(%f, 0.f, %f), global_item_type_data + ITEM_TYPE_wooden_sword);\n",
                                        x*TILE_SIZE + TILE_SIZE/2, y*TILE_SIZE + TILE_SIZE/2);
                                break;
                            }
                            
                            case '&':
                            {
                                fprintf(file, "TreasureChestSetAdd(&entities->treasure_chest_set, v3(%f, 0.f, %f), global_item_type_data + ITEM_TYPE_bomb);\n",
                                        x*TILE_SIZE + TILE_SIZE/2, y*TILE_SIZE + TILE_SIZE/2);
                                break;
                            }
                            
                            case 'G':
                            {
                                fprintf(file, "GoblinSetAddDefault(&entities->goblin_set, v3(%f, 0.f, %f));\n",
                                        x*TILE_SIZE + TILE_SIZE/2, y*TILE_SIZE + TILE_SIZE/2);
                                break;
                            }
                            
                            case 'W':
                            {
                                fprintf(file, "DarkWizardSetAddDefault(&entities->dark_wizard_set, v3(%f, 0.f, %f));\n",
                                        x*TILE_SIZE + TILE_SIZE/2, y*TILE_SIZE + TILE_SIZE/2);
                                break;
                            }
                            
                            case 'K':
                            {
                                fprintf(file, "KnightSetAddDefault(&entities->knight_set, v3(%f, 0.f, %f));\n",
                                        x*TILE_SIZE + TILE_SIZE/2, y*TILE_SIZE + TILE_SIZE/2);
                                break;
                            }
                            
                            case 'C':
                            {
                                fprintf(file, "CrateSetAddDefault(&entities->crate_set, v3(%f, 0.f, %f));\n",
                                        x*TILE_SIZE + TILE_SIZE/2, y*TILE_SIZE + TILE_SIZE/2);
                                break;
                            }
                            
                            default: break;
                        }
                    }
                }
            }
            fprintf(file, "}\n\n");
        }
    }
    
    fprintf(file, "global LoadedChunk global_%s_map_chunks[] = {\n", name);
    for(int chunk_y = 0; chunk_y < max_chunks_y; ++chunk_y)
    {
        for(int chunk_x = 0; chunk_x < max_chunks_x; ++chunk_x)
        {
            fprintf(file, "{ %i, %i, global_%s_chunk_%i%i_tiles, global_%s_chunk_%i%i_heights, global_%s_chunk_%i%i_vertex_data, ArrayCount(global_%s_chunk_%i%i_vertex_data), SpawnEntitiesForChunk_%s_%i%i },\n",
                    CHUNK_WIDTH_IN_TILES, CHUNK_HEIGHT_IN_TILES,
                    name, chunk_x, chunk_y,
                    name, chunk_x, chunk_y,
                    name, chunk_x, chunk_y,
                    name, chunk_x, chunk_y,
                    name, chunk_x, chunk_y);
        }
    }
    fprintf(file, "};\n\n");
    
    fprintf(file, "global LoadedMap global_%s_map = {\n", name);
    fprintf(file, "%i, %i, global_%s_map_chunks, { %i, %i }, \n",
            max_chunks_x, max_chunks_y, name, player_spawn_x, player_spawn_y);
    fprintf(file, "};\n\n");
}

DATA_DESK_FUNC void
DataDeskCustomDeclarationCallback(DataDeskDeclaration declaration_info, char *filename)
{
    if(CStringContains(filename, "gl_shader_"))
    {
        if(global_current_shader_filename != filename || !global_current_shader)
        {
            global_current_shader = global_shaders + global_shader_count++;
            global_current_shader_filename = filename;
        }
        
        if(CStringMatchCaseInsensitive(declaration_info.name, "vert"))
        {
            global_current_shader->vert = declaration_info.root;
        }
        else if(CStringMatchCaseInsensitive(declaration_info.name, "frag"))
        {
            global_current_shader->frag = declaration_info.root;
        }
    }
    else if(DataDeskDeclarationHasTag(declaration_info, "Map"))
    {
        FILE *file = global_map_header_file;
        FWriteMapData(file, declaration_info.root->declaration.initialization->string,
                      declaration_info.name);
    }
}

static int
GenerateWriteToDiskCodeForDeclaration(FILE *file, DataDeskASTNode *root, char *access_string,
                                      int access_string_max, int access_string_size,
                                      char *loop_string, char *index_string)
{
    int expected_size = 0;
    
    DataDeskASTNode *member = root;
    
    if(!DataDeskNodeHasTag(member, "DoNotSave"))
    {
        
        int loop_printed = 0;
#define PrintLoopIfNeeded() if(loop_string)\
        {\
            fprintf(file, "%s", loop_string);\
            fprintf(file, "{\n");\
            loop_printed = 1;\
        }
        
        if(DataDeskDeclarationIsType(member, "v4"))
        {
            PrintLoopIfNeeded();
            fprintf(file, "WriteMemory(&%.*s%s%s.x, sizeof(f32));\n", access_string_size, access_string,
                    member->string, index_string);
            fprintf(file, "WriteMemory(&%.*s%s%s.y, sizeof(f32));\n", access_string_size, access_string,
                    member->string, index_string);
            fprintf(file, "WriteMemory(&%.*s%s%s.z, sizeof(f32));\n", access_string_size, access_string,
                    member->string, index_string);
            fprintf(file, "WriteMemory(&%.*s%s%s.w, sizeof(f32));\n", access_string_size, access_string,
                    member->string, index_string);
            expected_size += sizeof(f32)*4;
        }
        else if(DataDeskDeclarationIsType(member, "f32"))
        {
            PrintLoopIfNeeded();
            fprintf(file, "WriteMemory(&%.*s%s%s, sizeof(f32));\n", access_string_size, access_string,
                    member->string, index_string);
            expected_size += sizeof(f32);
        }
        else if(DataDeskDeclarationIsType(member, "v2"))
        {
            PrintLoopIfNeeded();
            fprintf(file, "WriteMemory(&%.*s%s%s.x, sizeof(f32));\n", access_string_size, access_string,
                    member->string, index_string);
            fprintf(file, "WriteMemory(&%.*s%s%s.y, sizeof(f32));\n", access_string_size, access_string,
                    member->string, index_string);
            expected_size += sizeof(f32)*2;
        }
        else if(DataDeskDeclarationIsType(member, "iv2"))
        {
            PrintLoopIfNeeded();
            fprintf(file, "WriteMemory(&%.*s%s%s.x, sizeof(i32));\n", access_string_size, access_string,
                    member->string, index_string);
            fprintf(file, "WriteMemory(&%.*s%s%s.y, sizeof(i32));\n", access_string_size, access_string,
                    member->string, index_string);
            expected_size += sizeof(i32)*2;
        }
        else if(DataDeskDeclarationIsType(member, "b32"))
        {
            PrintLoopIfNeeded();
            fprintf(file, "WriteMemory(&%.*s%s%s, sizeof(b32));\n", access_string_size, access_string,
                    member->string, index_string);
            expected_size += sizeof(b32);
        }
        else if(DataDeskDeclarationIsType(member, "i32"))
        {
            PrintLoopIfNeeded();
            fprintf(file, "WriteMemory(&%.*s%s%s, sizeof(i32));\n", access_string_size, access_string,
                    member->string, index_string);
            expected_size += sizeof(i32);
        }
        else
        {
            for(int j = 0; j < global_component_count; ++j)
            {
                Component *component = global_components + j;
                if(CStringMatchCaseSensitive(component->name, member->declaration.type->string))
                {
                    if(!DataDeskNodeHasTag(component->root, "DoNotSave"))
                    {
                        PrintLoopIfNeeded();
                        char new_access_string[128] = {0};
                        int new_access_string_size = snprintf(new_access_string, sizeof(new_access_string),
                                                              "%.*s%s%s.", access_string_size, access_string,
                                                              member->string, index_string);
                        
                        for(DataDeskASTNode *component_member = component->root->struct_declaration.first_member;
                            component_member;
                            component_member = component_member->next)
                        {
                            expected_size += GenerateWriteToDiskCodeForDeclaration(file, component_member,
                                                                                   new_access_string, sizeof(new_access_string), new_access_string_size,
                                                                                   0, "");
                        }
                        
                    }
                    goto end_write_gen;
                }
            }
            
            end_write_gen:;
        }
        
#undef PrintLoopIfNeeded
        
        if(loop_string && loop_printed)
        {
            fprintf(file, "}\n");
        }
        
    }
    
    return expected_size;
}

static int
GenerateUICallForNode(FILE *file, DataDeskASTNode *node, char *target_format, ...)
{
    int success = 0;
    
    char target[128] = {0};
    va_list args;
    va_start(args, target_format);
    vsnprintf(target, sizeof(target), target_format, args);
    va_end(args);
    
    char label[128] = {0};
    if(DataDeskNodeHasTag(node, "Label(\""))
    {
        char *label_tag_str = DataDeskGetTagStringWithSubString(node, "Label(\"");
        label_tag_str += CStringFirstIndexAfterSubstring(label_tag_str, "Label(\"");
        int label_tag_str_len = 0;
        for(; label_tag_str[label_tag_str_len] && label_tag_str[label_tag_str_len] != '"';
            ++label_tag_str_len);
        snprintf(label, sizeof(label), "%.*s", label_tag_str_len, label_tag_str);
    }
    else
    {
        snprintf(label, sizeof(label), "%s", node->string);
    }
    
    if(!DataDeskNodeHasTag(node, "NoUI"))
    {
        if(DataDeskNodeHasTag(node, "ColorPicker"))
        {
            fprintf(file, "%s = UIEditorColorPicker(&core->ui, \"%s\", %s);\n",
                    target, label, target);
            goto end_node;
        }
        else if(DataDeskNodeHasTag(node, "NotePicker"))
        {
            fprintf(file, "%s = UIEditorNotePicker(&core->ui, \"%s\", %s);\n",
                    target, label, target);
            goto end_node;
        }
        else if(DataDeskNodeHasTag(node, "Slider("))
        {
            char *first_digit = DataDeskGetTagStringWithSubString(node, "Slider(");
            first_digit += CStringFirstIndexAfterSubstring(first_digit, "Slider(");
            char *second_digit = first_digit;
            second_digit += CStringFirstIndexAfterSubstring(second_digit, ",");
            float low = GetFirstF32FromCString(first_digit);
            float high = GetFirstF32FromCString(second_digit);
            fprintf(file, "%s = UIEditorSlider(&core->ui, \"%s\", %s, %f, %f);\n",
                    target, label, target, low, high);
            goto end_node;
        }
        else if(DataDeskDeclarationIsType(node, "b8")  ||
                DataDeskDeclarationIsType(node, "b16") ||
                DataDeskDeclarationIsType(node, "b32") ||
                DataDeskDeclarationIsType(node, "b64"))
        {
            fprintf(file, "%s = UIEditorToggler(&core->ui, \"%s\", %s);\n",
                    target, label, target);
            goto end_node;
        }
        else if(DataDeskDeclarationIsType(node, "char") &&
                node->declaration.type->type_usage.first_array_size_expression &&
                !node->declaration.type->type_usage.first_array_size_expression->next)
        {
            fprintf(file, "UIEditorLineEdit(&core->ui, \"%s\", %s, ",
                    label, target);
            DataDeskFWriteASTFromRootAsC(file, node->declaration.type->type_usage.first_array_size_expression, 0);
            fprintf(file, ");\n");
            goto end_node;
        }
        
        for(int j = 0; j < global_component_count; ++j)
        {
            if(CStringMatchCaseInsensitive(global_components[j].name,
                                           node->declaration.type->string))
            {
                
                for(DataDeskASTNode *component_member = global_components[j].root->struct_declaration.first_member;
                    component_member; component_member = component_member->next)
                {
                    success &= GenerateUICallForNode(file, component_member, "%s.%s",
                                                     target, component_member->string);
                }
                
                goto end_node;
            }
        }
        
        goto end_node_fail;
        
        end_node:;
        success = 1;
        goto end_node_all;
        end_node_fail:;
        success = 0;
        end_node_all:;
    }
    
    return success;
}

static void
FWriteMultilineStringConstantAsCStringConstant(FILE *file, char *string)
{
    fprintf(file, "\"");
    for(int i = 3; string[i]; ++i)
    {
        if(string[i] == '"' && string[i+1] == '"' && string[i+2] == '"')
        {
            break;
        }
        else if(string[i] == '\n')
        {
            fprintf(file, "\\n\"\n\"");
        }
        else
        {
            fprintf(file, "%c", string[i]);
        }
    }
    fprintf(file, "\"");
}

DATA_DESK_FUNC void
DataDeskCustomCleanUpCallback(void)
{
    // NOTE(rjf): Generate entity header code
    {
        
        FILE *file = global_entity_header_file;
        if(file)
        {
            fprintf(file, "typedef struct Map Map;\n");
            fprintf(file, "typedef struct Player Player;\n");
            fprintf(file, "typedef struct ParticleMaster ParticleMaster;\n");
            fprintf(file, "typedef struct ProjectileSet ProjectileSet;\n");
            for(int i = 0; i < global_entity_count; ++i)
            {
                Entity *entity = global_entities + i;
                
                fprintf(file, "#define %s_MAX %i\n", entity->name_uppercase_with_underscores, entity->max_count);
                fprintf(file, "#define %s_TYPE_ID %i\n", entity->name_uppercase_with_underscores, entity->entity_type_hash);
                
                // NOTE(rjf): Generate struct definition
                {
                    fprintf(file, "typedef struct %sSet\n{\n", entity->name_upper_camel_case);
                    
                    fprintf(file, "u32 count;\n");
                    fprintf(file, "u32 max_count;\n");
                    fprintf(file, "u32 *id_to_index_table;\n");
                    fprintf(file, "u32 *index_to_id_table;\n");
                    fprintf(file, "u32 free_id_count;\n");
                    fprintf(file, "u32 *free_id_list;\n");
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        fprintf(file, "%s *", member->declaration.type->string);
                        
                        for(int i = 0; i < member->declaration.type->type_usage.pointer_count; ++i)
                        {
                            fprintf(file, "*");
                        }
                        
                        fprintf(file, "%s;\n", member->string);
                    }
                    
                    fprintf(file, "}\n%sSet;\n\n", entity->name_upper_camel_case);
                }
                
                fprintf(file, "void %sSetInit(%sSet *set, u32 maximum, MemoryArena *arena);\n",
                        entity->name_upper_camel_case, entity->name_upper_camel_case);
                
                fprintf(file, "void %sSetCleanUp(%sSet *set);\n",
                        entity->name_upper_camel_case, entity->name_upper_camel_case);
                
                fprintf(file, "u32 %sSetGetNewID(%sSet *set);\n",
                        entity->name_upper_camel_case, entity->name_upper_camel_case);
                
                fprintf(file, "u32 %sSetDevelopmentMatchesMapFileEntity(char *name);\n",
                        entity->name_upper_camel_case);
                
                fprintf(file, "int %sSetAdd(%sSet *set, v3 spawn_position",
                        entity->name_upper_camel_case,
                        entity->name_upper_camel_case);
                
                for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                    member;
                    member = member->next)
                {
                    if(!DataDeskNodeHasTag(member, "Zero") &&
                       !DataDeskNodeHasTag(member, "Update"))
                    {
                        fprintf(file, ", ");
                        DataDeskFWriteASTFromRootAsC(file, member, 0);
                    }
                }
                
                fprintf(file, ");\n");
                
                fprintf(file, "int %sSetAddDefault(%sSet *set, v3 spawn_position);\n",
                        entity->name_upper_camel_case,
                        entity->name_upper_camel_case);
                
                fprintf(file, "int %sSetRemoveByIndex(%sSet *set, u32 index);\n",
                        entity->name_upper_camel_case, entity->name_upper_camel_case);
                
                fprintf(file, "int %sSetRemoveByID(%sSet *set, u32 id);\n",
                        entity->name_upper_camel_case, entity->name_upper_camel_case);
                
                fprintf(file, "void %sSetUpdate(%sSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles);\n", entity->name_upper_camel_case,
                        entity->name_upper_camel_case);
                
                fprintf(file, "void %sSetRender(%sSet *set);\n",
                        entity->name_upper_camel_case, entity->name_upper_camel_case);
                
                fprintf(file, "\n");
                
            }
            
            fprintf(file, "typedef struct EntitySets\n{\n");
            for(int i = 0; i < global_entity_count; ++i)
            {
                Entity *entity = global_entities + i;
                fprintf(file, "%sSet %s_set;\n",
                        entity->name, entity->name_lowercase_with_underscores);
            }
            fprintf(file, "}\nEntitySets;\n\n");
            
            fprintf(file, "void EntitySetsInit(EntitySets *entities, MemoryArena *arena);\n");
            fprintf(file, "void EntitySetsCleanUp(EntitySets *entities);\n");
            fprintf(file, "typedef struct ParticleMaster ParticleMaster;\n");
            fprintf(file, "void EntitySetsDoDefaultUpdate(EntitySets *entities, Map *map, Player *player, ProjectileSet *projectiles, ParticleMaster *particles);\n");
            fprintf(file, "void EntitySetsRender(EntitySets *entities);\n");
            fprintf(file, "void EntitySetsRemoveByID(EntitySets *entities, EntityID id);\n");
        }
        
    }
    
    // NOTE(rjf): Generate entity implementation code
    {
        
        FILE *file = global_entity_implementation_file;
        if(file)
        {
            
            // NOTE(rjf): Entity sets init
            {
                fprintf(file, "void EntitySetsInit(EntitySets *entities, MemoryArena *arena)\n{\n");
                
                for(int i = 0; i < global_entity_count; ++i)
                {
                    fprintf(file, "%sSetInit(&entities->%s_set, %s_MAX, arena);\n", global_entities[i].name,
                            global_entities[i].name_lowercase_with_underscores,
                            global_entities[i].name_uppercase_with_underscores);
                }
                
                fprintf(file, "}\n\n");
            }
            
            // NOTE(rjf): Entity sets clean up
            {
                fprintf(file, "void EntitySetsCleanUp(EntitySets *entities)\n{\n");
                
                for(int i = 0; i < global_entity_count; ++i)
                {
                    fprintf(file, "%sSetCleanUp(&entities->%s_set);\n", global_entities[i].name,
                            global_entities[i].name_lowercase_with_underscores);
                }
                
                fprintf(file, "}\n\n");
            }
            
            // NOTE(rjf): Entity sets default update
            {
                fprintf(file, "internal void CollideHealthAndSphereComponentsWithAttackComponents(HealthComponent *health, SphereComponent *sphere, u32 count, AttackComponent *attack, u32 attack_count, ParticleMaster *particles);\n");
                fprintf(file, "void EntitySetsDoDefaultUpdate(EntitySets *entities, Map *map, Player *player, ProjectileSet *projectiles, ParticleMaster *particles)\n{\n");
                
                for(int i = 0; i < global_entity_count; ++i)
                {
                    fprintf(file, "%sSetUpdate(&entities->%s_set, map, particles, player, projectiles);\n", global_entities[i].name,
                            global_entities[i].name_lowercase_with_underscores);
                }
                
                // NOTE(rjf): Update health components on all attack components
                {
                    fprintf(file, "struct\n{\nAttackComponent *attack; u32 count;\n}\nattack_set_list[] = {\n");
                    fprintf(file, "{ &player->attack, 1, },\n");
                    for(int i = 0; i < global_entity_count; ++i)
                    {
                        Entity *entity = global_entities+i;
                        for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                            member; member = member->next)
                        {
                            if(DataDeskDeclarationIsType(member, "AttackComponent"))
                            {
                                fprintf(file, "{ entities->%s_set.%s, entities->%s_set.count, },\n",
                                        entity->name_lowercase_with_underscores, member->string,
                                        entity->name_lowercase_with_underscores);
                                break;
                            }
                        }
                    }
                    fprintf(file, "};\n\n");
                    
                    for(int i = 0; i < global_entity_count; ++i)
                    {
                        Entity *entity = global_entities+i;
                        
                        DataDeskASTNode *health_component = 0;
                        DataDeskASTNode *sphere_component = 0;
                        DataDeskASTNode *attack_component = 0;
                        
                        for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                            member; member = member->next)
                        {
                            if(DataDeskDeclarationIsType(member, "HealthComponent"))
                            {
                                health_component = member;
                            }
                            else if(DataDeskDeclarationIsType(member, "SphereComponent"))
                            {
                                sphere_component = member;
                            }
                            else if(DataDeskDeclarationIsType(member, "AttackComponent"))
                            {
                                attack_component = member;
                            }
                        }
                        
                        if(health_component && sphere_component)
                        {
                            fprintf(file, "for(u32 i = 0; i < ArrayCount(attack_set_list); ++i)\n{\n");
                            fprintf(file, "CollideHealthAndSphereComponentsWithAttackComponents(entities->%s_set.%s, entities->%s_set.%s, entities->%s_set.count, attack_set_list[i].attack, attack_set_list[i].count, particles);\n",
                                    entity->name_lowercase_with_underscores, health_component->string,
                                    entity->name_lowercase_with_underscores, sphere_component->string,
                                    entity->name_lowercase_with_underscores);
                            fprintf(file, "}\n\n");
                            
                            fprintf(file, "CollideHealthAndSphereComponentsWithProjectiles(entities->%s_set.%s, entities->%s_set.%s, entities->%s_set.count, projectiles, particles);\n",
                                    entity->name_lowercase_with_underscores, health_component->string,
                                    entity->name_lowercase_with_underscores, sphere_component->string,
                                    entity->name_lowercase_with_underscores);
                        }
                        
                        if(attack_component)
                        {
                            fprintf(file, "CollideHealthAndSphereComponentsWithAttackComponents(&player->health, &player->sphere, 1, entities->%s_set.%s, entities->%s_set.count, particles);\n",
                                    entity->name_lowercase_with_underscores, attack_component->string,
                                    entity->name_lowercase_with_underscores);
                        }
                        
                    }
                }
                
                fprintf(file, "}\n\n");
            }
            
            // NOTE(rjf): Entity sets render
            {
                fprintf(file, "void EntitySetsRender(EntitySets *entities)\n{\n");
                
                for(int i = 0; i < global_entity_count; ++i)
                {
                    fprintf(file, "%sSetRender(&entities->%s_set);\n", global_entities[i].name,
                            global_entities[i].name_lowercase_with_underscores,
                            global_entities[i].name_lowercase_with_underscores);
                }
                
                fprintf(file, "}\n\n");
            }
            
            // NOTE(rjf): Entity sets remove by ID
            {
                fprintf(file, "void EntitySetsRemoveByID(EntitySets *entities, EntityID id)\n{\n");
                
                for(int i = 0; i < global_entity_count; ++i)
                {
                    Entity *entity = global_entities + i;
                    fprintf(file, "if(id.type == %s_TYPE_ID)\n{\n",
                            entity->name_uppercase_with_underscores);
                    fprintf(file, "%sSetRemoveByID(&entities->%s_set, entities->%s_set.id_to_index_table[id.instance_id]);\n",
                            entity->name_upper_camel_case, entity->name_lowercase_with_underscores, entity->name_lowercase_with_underscores);
                    fprintf(file, "}\nelse\n");
                }
                fprintf(file, "{}\n");
                
                fprintf(file, "}\n\n");
            }
            
            for(int i = 0; i < global_entity_count; ++i)
            {
                Entity *entity = global_entities + i;
                
                DataDeskASTNode *sphere_component = 0;
                DataDeskASTNode *static_float_component = 0;
                DataDeskASTNode *sprite_component = 0;
                DataDeskASTNode *weapon_component = 0;
                DataDeskASTNode *enemy_ai_component = 0;
                DataDeskASTNode *attack_component = 0;
                DataDeskASTNode *health_component = 0;
                
                for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                    member;
                    member = member->next)
                {
                    if(DataDeskDeclarationIsType(member, "SphereComponent"))
                    {
                        sphere_component = member;
                    }
                    else if(DataDeskDeclarationIsType(member, "SpriteComponent"))
                    {
                        sprite_component = member;
                    }
                    else if(DataDeskDeclarationIsType(member, "StaticFloatComponent"))
                    {
                        static_float_component = member;
                    }
                    else if(DataDeskDeclarationIsType(member, "EnemyAIComponent"))
                    {
                        enemy_ai_component = member;
                    }
                    else if(DataDeskDeclarationIsType(member, "AttackComponent"))
                    {
                        attack_component = member;
                    }
                    else if(DataDeskDeclarationIsType(member, "HealthComponent"))
                    {
                        health_component = member;
                    }
                    else if(DataDeskDeclarationIsType(member, "WeaponComponent"))
                    {
                        weapon_component = member;
                    }
                }
                
                // NOTE(rjf): Generate initialization function
                {
                    fprintf(file, "void %sSetInit(%sSet *set, u32 maximum, MemoryArena *arena)\n{\n",
                            entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    fprintf(file, "set->count = 0;\n");
                    fprintf(file, "set->max_count = maximum;\n");
                    
                    fprintf(file, "set->id_to_index_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);\n");
                    fprintf(file, "set->index_to_id_table = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);\n");
                    fprintf(file, "set->free_id_count = maximum;\n");
                    fprintf(file, "set->free_id_list = (u32 *)MemoryArenaAllocate(arena, sizeof(u32) * maximum);\n\n");
                    
                    fprintf(file, "for(u32 i = 0; i < maximum; ++i)\n{\n");
                    fprintf(file, "set->free_id_list[i] = maximum - i - 1;\n");
                    fprintf(file, "}\n\n");
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        fprintf(file, "set->%s = (", member->string);
                        fprintf(file, "%s", member->declaration.type->string);
                        for(int i = 0; i < member->declaration.type->type_usage.pointer_count+1; ++i)
                        {
                            if(!i)
                            {
                                fprintf(file, " ");
                            }
                            fprintf(file, "*");
                        }
                        fprintf(file, ")MemoryArenaAllocate(arena, sizeof(set->%s[0]) * maximum);\n", member->string);
                    }
                    
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate clean-up function
                {
                    fprintf(file, "void %sSetCleanUp(%sSet *set)\n{\n",
                            entity->name_upper_camel_case, entity->name_upper_camel_case);
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate ID gen function
                {
                    fprintf(file, "u32 %sSetGetNewID(%sSet *set)\n{\n",
                            entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    
                    fprintf(file, "u32 id = 0;\n");
                    fprintf(file, "if(set->free_id_count > 0)\n{\n");
                    fprintf(file, "--set->free_id_count;\n");
                    fprintf(file, "id = set->free_id_list[set->free_id_count];\n");
                    fprintf(file, "}\n");
                    fprintf(file, "return id;\n");
                    
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate string matching function
                {
                    fprintf(file, "u32 %sSetDevelopmentMatchesMapFileEntity(char *name)\n{\n",
                            entity->name_upper_camel_case);
                    fprintf(file, "return CStringMatchCaseInsensitive(name, \"%s\");\n",
                            entity->name_with_spaces);
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate initialization
                {
                    fprintf(file, "int %sSetInitInstanceByIndex(%sSet *set, u32 index, v3 spawn_position",
                            entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        if(!DataDeskNodeHasTag(member, "Zero") &&
                           !DataDeskNodeHasTag(member, "Update"))
                        {
                            fprintf(file, ", ");
                            DataDeskFWriteASTFromRootAsC(file, member, 0);
                        }
                    }
                    
                    fprintf(file, ")\n{\n");
                    fprintf(file, "int success = 0;\n");
                    fprintf(file, "if(index < set->max_count)\n{\n");
                    fprintf(file, "success = 1;\n");
                    
                    fprintf(file, "EntityID id = { (u16)%s_TYPE_ID, (u16)index, };\n",
                            entity->name_uppercase_with_underscores);
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        if(DataDeskNodeHasTag(member, "Zero"))
                        {
                            DataDeskFWriteASTFromRootAsC(file, member, 0);
                            fprintf(file, " = {0};\n");
                        }
                        else if(DataDeskNodeHasTag(member, "Update"))
                        {
                            DataDeskFWriteASTFromRootAsC(file, member, 0);
                            fprintf(file, " = ");
                            char *init_string = DataDeskGetTagStringWithSubString(member, "Update(\"");
                            if(init_string)
                            {
                                init_string += CStringFirstIndexAfterSubstring(init_string, "Update(\"");
                                int string_length = 0;
                                for(; init_string[string_length] && init_string[string_length] != '"';
                                    ++string_length);
                                fprintf(file, "%.*s;\n", string_length, init_string);
                            }
                            else
                            {
                                fprintf(file, "{0};\n");
                            }
                        }
                        
                        fprintf(file, "set->%s[index] = %s;\n",
                                member->string, member->string);
                    }
                    
                    fprintf(file, "}\n");
                    fprintf(file, "return success;\n");
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate adding function
                {
                    fprintf(file, "int %sSetAdd(%sSet *set, v3 spawn_position",
                            entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        if(!DataDeskNodeHasTag(member, "Zero") &&
                           !DataDeskNodeHasTag(member, "Update"))
                        {
                            fprintf(file, ", ");
                            DataDeskFWriteASTFromRootAsC(file, member, 0);
                        }
                    }
                    
                    fprintf(file, ")\n{\n");
                    fprintf(file, "int success = 0;\n");
                    fprintf(file, "if(set->count < set->max_count)\n{\n");
                    fprintf(file, "success = 1;\n");
                    fprintf(file, "u32 index = set->count++;\n");
                    fprintf(file, "u32 id = %sSetGetNewID(set);\n", entity->name_upper_camel_case);
                    fprintf(file, "set->index_to_id_table[index] = id;\n");
                    fprintf(file, "set->id_to_index_table[id] = index;\n");
                    
                    fprintf(file, "%sSetInitInstanceByIndex(set, index, spawn_position", entity->name_upper_camel_case);
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        if(!DataDeskNodeHasTag(member, "Zero") &&
                           !DataDeskNodeHasTag(member, "Update"))
                        {
                            fprintf(file, ", ");
                            fprintf(file, "%s", member->string);
                        }
                    }
                    
                    fprintf(file, ");\n");
                    
                    fprintf(file, "}\n");
                    fprintf(file, "return success;\n");
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate default adding function
                {
                    fprintf(file, "int %sSetAddDefault(%sSet *set, v3 spawn_position)\n{\n",
                            entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        if(!DataDeskNodeHasTag(member, "Zero") &&
                           !DataDeskNodeHasTag(member, "Update") &&
                           DataDeskNodeHasTag(member, "Default"))
                        {
                            DataDeskFWriteASTFromRootAsC(file, member, 0);
                            fprintf(file, " = ");
                            char *default_str = DataDeskGetTagStringWithSubString(member, "Default(\"");
                            default_str += CStringFirstIndexAfterSubstring(default_str, "Default(\"");
                            int default_str_len = 0;
                            for(; default_str[default_str_len] && default_str[default_str_len] != '"'; ++default_str_len);
                            fprintf(file, "%.*s", default_str_len, default_str);
                            fprintf(file, ";");
                        }
                    }
                    
                    fprintf(file, "return %sSetAdd(set, spawn_position",
                            entity->name_upper_camel_case);
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        if(!DataDeskNodeHasTag(member, "Zero") &&
                           !DataDeskNodeHasTag(member, "Update") &&
                           DataDeskNodeHasTag(member, "Default"))
                        {
                            fprintf(file, ", %s", member->string);
                        }
                    }
                    
                    fprintf(file, ");\n");
                    
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate removal by index function
                {
                    fprintf(file, "int %sSetRemoveByIndex(%sSet *set, u32 index)\n{\n",
                            entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    
                    fprintf(file, "int success = 0;\n\n");
                    fprintf(file, "if(index >= 0 && index < set->count)\n{\n");
                    fprintf(file, "success = 1;\n");
                    fprintf(file, "if(index != --set->count)\n{\n");
                    fprintf(file, "set->free_id_list[set->free_id_count++] = set->index_to_id_table[index];\n");
                    fprintf(file, "u32 replacement_index = set->count;\n");
                    fprintf(file, "u32 replacement_id = set->index_to_id_table[replacement_index];\n");
                    fprintf(file, "set->id_to_index_table[replacement_id] = replacement_index;\n");
                    fprintf(file, "set->index_to_id_table[replacement_index] = replacement_id;\n");
                    
                    for(DataDeskASTNode *member = entity->root->struct_declaration.first_member;
                        member;
                        member = member->next)
                    {
                        fprintf(file, "set->%s[index] = set->%s[replacement_index];\n",
                                member->string, member->string);
                    }
                    
                    fprintf(file, "}\n\n");
                    fprintf(file, "}\n\n");
                    fprintf(file, "return success;\n");
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate removal by ID function
                {
                    fprintf(file, "int %sSetRemoveByID(%sSet *set, u32 id)\n{\n",
                            entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    fprintf(file, "return %sSetRemoveByIndex(set, set->id_to_index_table[id]);\n",
                            entity->name_upper_camel_case);
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate update function
                {
                    fprintf(file, "void %sSetUpdate(%sSet *set, Map *map, ParticleMaster *particles, Player *player, ProjectileSet *projectiles)\n{\n",
                            entity->name_upper_camel_case, entity->name_upper_camel_case);
                    
                    if(sphere_component)
                    {
                        fprintf(file, "UpdateSphereComponents(set->%s, map, set->count);\n",
                                sphere_component->string);
                        
                        if(sprite_component)
                        {
                            fprintf(file, "TrackSpriteComponentsToSphereComponents(set->%s, set->%s, set->count, map);\n",
                                    sprite_component->string, sphere_component->string);
                        }
                        
                        if(weapon_component)
                        {
                            fprintf(file, "TrackWeaponComponentsToSphereComponents(set->%s, set->%s, set->count);\n",
                                    weapon_component->string, sphere_component->string);
                        }
                    }
                    else if(static_float_component)
                    {
                        fprintf(file, "UpdateStaticFloatComponents(set->%s, map, set->count);\n",
                                static_float_component->string);
                        
                        if(sprite_component)
                        {
                            fprintf(file, "TrackSpriteComponentsToStaticFloatComponents(set->%s, set->%s, set->count);\n",
                                    sprite_component->string, static_float_component->string);
                        }
                        
                    }
                    
                    if(sprite_component)
                    {
                        fprintf(file, "UpdateSpriteComponents(set->%s, set->count);\n",
                                sprite_component->string);
                    }
                    
                    if(enemy_ai_component)
                    {
                        if(attack_component)
                        {
                            fprintf(file, "UpdateEnemyAI(set->%s, set->%s, set->count, player);\n",
                                    enemy_ai_component->string,
                                    attack_component->string);
                        }
                        
                        if(sphere_component)
                        {
                            fprintf(file, "TrackEnemyAIComponentsToSphereComponents(set->%s, set->%s, set->count);\n",
                                    enemy_ai_component->string, sphere_component->string);
                            fprintf(file, "TrackSphereComponentsToEnemyAIComponents(set->%s, set->%s, set->count);\n",
                                    sphere_component->string, enemy_ai_component->string);
                        }
                    }
                    
                    if(attack_component)
                    {
                        fprintf(file, "UpdateAttackComponents(set->%s, set->count, projectiles);\n",
                                attack_component->string);
                        
                        if(sphere_component)
                        {
                            fprintf(file, "TrackAttackComponentsToSphereComponents(set->%s, set->%s, set->count);\n",
                                    attack_component->string, sphere_component->string);
                        }
                    }
                    
                    if(health_component)
                    {
                        fprintf(file, "UpdateHealthComponents(set->%s, set->count);\n",
                                health_component->string);
                        
                        if(sphere_component)
                        {
                            fprintf(file, "TrackHealthComponentsToSphereComponents(set->%s, set->%s, set->count);\n",
                                    health_component->string, sphere_component->string);
                        }
                        
                        if(sprite_component)
                        {
                            fprintf(file, "TrackSpriteComponentsToHealthComponents(set->%s, set->%s, set->count);\n",
                                    sprite_component->string, health_component->string);
                        }
                        
                        fprintf(file, "for(u32 i = 0; i < set->count;)\n{\n");
                        fprintf(file, "    if(set->%s[i].health <= 0.f)\n{\n", health_component->string);
                        fprintf(file, "        for(u32 j = 0; j < 100; ++j)\n{\n");
                        fprintf(file, "            ParticleSpawn(particles, PARTICLE_TYPE_strike, set->%s[i].position, v3(RandomF32(-1, 1), RandomF32(-1, 1), RandomF32(-1, 1)));\n",
                                health_component->string);
                        fprintf(file, "        }\n");
                        fprintf(file, "        %sSetRemoveByIndex(set, i);\n", entity->name_upper_camel_case);
                        fprintf(file, "    }\n");
                        fprintf(file, "    else\n{\n");
                        fprintf(file, "        ++i;\n");
                        fprintf(file, "    }\n");
                        fprintf(file, "}\n\n");
                    }
                    
                    fprintf(file, "}\n\n");
                }
                
                // NOTE(rjf): Generate render function
                {
                    fprintf(file, "void %sSetRender(%sSet *set)\n{\n", entity->name_upper_camel_case,
                            entity->name_upper_camel_case);
                    
                    if(sprite_component)
                    {
                        fprintf(file, "RenderSpriteComponents(set->%s, set->count);\n",
                                sprite_component->string);
                    }
                    
                    if(weapon_component)
                    {
                        fprintf(file, "RenderWeaponComponents(set->%s, set->count);\n",
                                weapon_component->string);
                    }
                    
                    if(sphere_component)
                    {
                        fprintf(file, "RenderSphereComponentsDebug(set->%s, set->count);\n",
                                sphere_component->string);
                    }
                    
                    if(attack_component)
                    {
                        fprintf(file, "RenderAttackComponentsDebug(set->%s, set->count);\n",
                                attack_component->string);
                    }
                    
                    fprintf(file, "}\n\n");
                }
            }
        }
    }
    
    // NOTE(rjf): Generate shader header code
    {
        FILE *file = global_shader_header_file;
        
        fprintf(file, "typedef enum OpenGLShaderType OpenGLShaderType;\n");
        fprintf(file, "enum OpenGLShaderType\n{\n");
        for(int i = 0; i < global_shader_count; ++i)
        {
            char *name = global_shaders[i].filename;
            name += CStringFirstIndexAfterSubstring(name, "gl_shader_");
            int name_length = 0;
            for(; name[name_length] && name[name_length] != '.'; ++name_length);
            fprintf(file, "OPENGL_SHADER_%.*s,\n", name_length, name);
        }
        fprintf(file, "OPENGL_SHADER_MAX,\n");
        fprintf(file, "};\n\n");
        
        fprintf(file, "#define OPENGL_SHADER_INPUT_MAX 16\n");
        fprintf(file, "#define OPENGL_SHADER_OUTPUT_MAX 16\n");
        fprintf(file, "global struct\n{\n");
        fprintf(file, "char *name;\n");
        fprintf(file, "OpenGLShaderInput inputs[OPENGL_SHADER_INPUT_MAX];\n");
        fprintf(file, "u32 input_count;\n");
        fprintf(file, "OpenGLShaderOutput outputs[OPENGL_SHADER_OUTPUT_MAX];\n");
        fprintf(file, "u32 output_count;\n");
        fprintf(file, "char *vert;\n");
        fprintf(file, "char *frag;\n");
        fprintf(file, "}\n");
        fprintf(file, "global_opengl_shaders[] = {\n");
        
        for(int i = 0; i < global_shader_count; ++i)
        {
            Shader *shader = global_shaders + i;
            
            if(shader->info && shader->vert && shader->frag)
            {
                
                fprintf(file, "{\n");
                fprintf(file, "\"%s\",\n", shader->name);
                
                int input_count = 0;
                
                fprintf(file, "{\n");
                for(DataDeskASTNode *first_tag = shader->info->first_tag;
                    first_tag;
                    first_tag = first_tag->next)
                {
                    if(CStringContains(first_tag->string, "Input("))
                    {
                        char *input = first_tag->string;
                        input += CStringFirstIndexAfterSubstring(input, "Input(");
                        input += CStringFirstIndexAfterSubstring(input, "\"");
                        
                        int name_length = 0;
                        char *name = input;
                        for(; input[name_length] && input[name_length] != '"'; ++name_length);
                        
                        input += CStringFirstIndexAfterSubstring(input, "\"");
                        input += CStringFirstIndexAfterSubstring(input, ",");
                        
                        char *index_str = input;
                        int index = GetFirstI32FromCString(index_str);
                        
                        fprintf(file, "{ %i, \"%.*s\", },\n", index, name_length, name);
                        
                        ++input_count;
                    }
                }
                
                if(!input_count)
                {
                    fprintf(file, "0");
                }
                
                fprintf(file, "},\n");
                
                fprintf(file, "%i,\n", input_count);
                
                int output_count = 0;
                
                fprintf(file, "{\n");
                for(DataDeskASTNode *first_tag = shader->info->first_tag;
                    first_tag;
                    first_tag = first_tag->next)
                {
                    if(CStringContains(first_tag->string, "Output("))
                    {
                        char *input = first_tag->string;
                        input += CStringFirstIndexAfterSubstring(input, "Output(");
                        input += CStringFirstIndexAfterSubstring(input, "\"");
                        
                        int name_length = 0;
                        char *name = input;
                        for(; input[name_length] && input[name_length] != '"'; ++name_length);
                        
                        input += CStringFirstIndexAfterSubstring(input, "\"");
                        input += CStringFirstIndexAfterSubstring(input, ",");
                        
                        char *index_str = input;
                        int index = GetFirstI32FromCString(index_str);
                        
                        fprintf(file, "{ %i, \"%.*s\", },\n", index, name_length, name);
                        
                        ++output_count;
                    }
                }
                
                if(!output_count)
                {
                    fprintf(file, "0");
                }
                
                fprintf(file, "},\n");
                
                fprintf(file, "%i,\n", output_count);
                
                fprintf(file, "\"#version 330 core\\n\"\n");
                FWriteMultilineStringConstantAsCStringConstant(file, shader->vert->declaration.initialization->string);
                fprintf(file, ",\n");
                
                fprintf(file, "\"#version 330 core\\n\"\n");
                FWriteMultilineStringConstantAsCStringConstant(file, shader->frag->declaration.initialization->string);
                fprintf(file, ",\n");
                
                fprintf(file, "},\n");
            }
        }
        
        fprintf(file, "};\n\n");
    }
    
    // NOTE(rjf): Generate shader implementation code
    {
        FILE *file = global_shader_implementation_file;
    }
}