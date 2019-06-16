
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

typedef struct String
{
    char *data;
    u32 length;
    u32 size;
    u32 max_size;
    b32 is_mutable;
}
String;

internal String
MakeStringOnMemoryArena(MemoryArena *arena, char *format, ...)
{
    String string = {0};
    
    va_list args;
    va_start(args, format);
    u32 needed_bytes = vsnprintf(0, 0, format, args)+1;
    va_end(args);
    
    string.data = (char *)MemoryArenaAllocate(arena, needed_bytes);
    SoftAssert(string.data);
    if(string.data)
    {
        string.length = needed_bytes-1;
        string.size = needed_bytes;
        string.max_size = string.size;
        string.is_mutable = 0;
        
        va_start(args, format);
        vsnprintf(string.data, needed_bytes, format, args);
        va_end(args);
        
        string.data[needed_bytes-1] = 0;
    }
    
    return string;
}

enum
{
    MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_null,
    MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_color,
    MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_reset_color,
    MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_speed,
    MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_reset_speed,
    MESSAGE_TEXT_DESCRIPTOR_PARSE_RESULT_MAX
};

typedef struct MessageTextDescriptorParseResult
{
    int type;
    
    union {
        struct
        {
            v3 color;
        } color;
        
        struct
        {
            f32 speed;
        } speed;
    };
}
MessageTextDescriptorParseResult;

internal MessageTextDescriptorParseResult
MessageTextDescriptorParse(char *descriptor);