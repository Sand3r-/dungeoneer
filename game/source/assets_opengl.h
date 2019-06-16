struct Texture
{
    i16 w, h;
    GLuint id;
    b32 loaded;
};

struct Font
{
    Texture texture;
    i16 size;
    i16 line_height;
    i16 char_x[95];
    i16 char_y[95];
    i16 char_w[95];
    i16 char_h[95];
    i16 char_x_offset[95];
    i16 char_y_offset[95];
    i16 char_x_advance[95];
};

#define MODEL_FLAG_POSITION 0x01
#define MODEL_FLAG_UV       0x02
#define MODEL_FLAG_NORMAL   0x04

struct Model
{
    i32 flags;
    GLuint vao;
    GLuint vertex_buffer;
    u32 vertex_count;
};