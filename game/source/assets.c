#if RENDERER_BACKEND == RENDERER_OPENGL
#include "assets_opengl.c"
#endif

internal b32
SoundIsLoaded(Sound *sound)
{
    return sound->sample_count && sound->samples != 0;
}

internal Sound
SoundInitFromOGGData(void *data, u64 len)
{
    Sound s = {0};
    s.sample_count = 
        stb_vorbis_decode_memory(
        (u8 *)data, (i32)len, &s.channels, &s.sample_rate, 
        &s.samples
        );
    s.sample_count *= s.channels;
    return s;
}

internal Sound
SoundLoad(char *name)
{
    Sound sound = {0};
    void *ogg_data = 0;
    u32 ogg_data_len = 0;
    
    char filename[512] = {0};
    snprintf(filename, sizeof(filename), "%s.ogg", name);
    
    platform->LoadEntireFile(filename, &ogg_data, &ogg_data_len);
    if(ogg_data && ogg_data_len)
    {
        sound = SoundInitFromOGGData(ogg_data, ogg_data_len);
        platform->FreeFileData(ogg_data);
    }
    return sound;
}

internal void
SoundCleanUp(Sound *s)
{
    if(s->samples)
    {
        free(s->samples);
        s->samples = 0;
        s->sample_count = 0;
    }
}
