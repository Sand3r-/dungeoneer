typedef struct Texture Texture;
typedef struct Font Font;
typedef struct Model Model;

#if RENDERER_BACKEND == RENDERER_OPENGL
#include "assets_opengl.h"
#endif

typedef struct Sound Sound;
struct Sound
{
    i32 channels;
    i32 sample_rate;
    i16 *samples;
    u32 sample_count;
};

// NOTE(rjf): API that must be filled out by various renderer backends:
struct Texture;
struct Font;
struct Model;
internal Texture TextureInitFromData(void *data, u32 len);
internal Texture TextureLoad(char *name);
internal void TextureCleanUp(Texture *texture);
internal Font FontInitFromData(void *image, u32 image_len, void *font, u32 font_len);
internal Font FontLoad(char *name);
internal void FontCleanUp(Font *font);
internal Model ModelInitFromData(f32 *data, u32 number_of_floats, i32 flags);
internal void ModelCleanUp(Model *model);
internal b32 TextureIsLoaded(Texture *texture);
internal b32 FontIsLoaded(Font *font);
internal b32 ModelIsLoaded(Model *model);
internal f32 FontGetTextWidth(Font *font, char *text);
internal f32 FontGetLineHeight(Font *font);

// NOTE(rjf): API that must be filled out by various build modes:
internal Sound SoundInitFromOggData(void *data, u32 len);
internal Sound SoundLoad(char *name);
internal void SoundCleanUp(Sound *sound);
internal b32 SoundIsLoaded(Sound *sound);