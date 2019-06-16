typedef struct Shader Shader;
struct Shader
{
    GLuint id;
};

#include "generated/generated_shaders.h"

#define RENDERER_OPENGL_FBO_COLOR_OUT_0  0x01
#define RENDERER_OPENGL_FBO_COLOR_OUT_1  0x02
#define RENDERER_OPENGL_FBO_COLOR_OUT_2  0x04
#define RENDERER_OPENGL_FBO_COLOR_OUT_3  0x08
#define RENDERER_OPENGL_FBO_DEPTH_OUT    0x10

typedef struct RendererOpenGLFBO RendererOpenGLFBO;
struct RendererOpenGLFBO
{
    i32 flags;
    GLuint fbo;
    GLuint color_textures[4];
    GLuint depth_texture;
    
    union
    {
        struct
        {
            u32 w;
            u32 h;
        };
        struct
        {
            u32 width;
            u32 height;
        };
    };
};

typedef struct RendererRequest RendererRequest;
struct RendererRequest
{
    RendererRequestType type;
    Texture *texture;
    u32 data_offset;
    u32 data_size;
    i32 flags;
    v4 color;
    union
    {
        struct
        {
            union
            {
                v4 clip;
                struct
                {
                    v4 blur_rect;
                    f32 blur_magnitude;
                };
            };
        };
        
        m4 view;
        m4 projection;
        
        struct
        {
            m4 model_matrix;
            GLuint model_vao;
            u32 model_vertex_count;
        };
    };
};

typedef struct Renderer Renderer;
struct Renderer
{
    RENDERER_COMMON_DATA;
    
    // NOTE(rjf): Frame Data
    f32 render_w;
    f32 render_h;
    
    // NOTE(rjf): All-purpose VAO
    GLuint all_purpose_vao;
    
    // NOTE(rjf): Line data
    GLuint line_vao;
    GLuint line_instance_buffer;
    u32 line_instance_data_alloc_pos;
    // NOTE(rjf):                       x,y,z,x,y,z,    r,g,b,a
#define RENDERER_OPENGL_BYTES_PER_LINE (sizeof(f32) * 10)
    GLubyte line_instance_data[RENDERER_MAX_LINES * RENDERER_OPENGL_BYTES_PER_LINE];
    
    // NOTE(rjf): Filled rectangle data
    GLuint filled_rect_vao;
    GLuint filled_rect_instance_buffer;
    u32 filled_rect_instance_data_alloc_pos;
    // NOTE(rjf):                              xyz,xyz,xyz,xyz, color (per vertex)
#define RENDERER_OPENGL_BYTES_PER_FILLED_RECT (sizeof(f32) * 28)
    GLubyte filled_rect_instance_data[RENDERER_MAX_FILLED_RECTS * RENDERER_OPENGL_BYTES_PER_FILLED_RECT];
    
    // NOTE(rjf): Texture data
    GLuint texture_vao;
    GLuint texture_instance_buffer;
    u32 texture_instance_data_alloc_pos;
    // NOTE(rjf):                          source, xyz,xyz,xyz,xyz, rgba
#define RENDERER_OPENGL_BYTES_PER_TEXTURE (sizeof(f32) * 20)
    GLubyte texture_instance_data[RENDERER_MAX_TEXTURES * RENDERER_OPENGL_BYTES_PER_TEXTURE];
    
    // NOTE(rjf): Text data
    GLuint text_vao;
    GLuint text_instance_buffer;
    u32 text_instance_data_alloc_pos;
    // NOTE(rjf):                        source, xyz,xyz,xyz,xyz, color, boldness, softness
#define RENDERER_OPENGL_BYTES_PER_TEXT (sizeof(f32) * 22)
    GLubyte text_instance_data[RENDERER_MAX_TEXTS * RENDERER_OPENGL_BYTES_PER_TEXT];
    
    // NOTE(rjf): Debug-sphere
    GLuint debug_sphere_vao;
    GLuint debug_sphere_vertex_buffer;
    GLuint debug_sphere_buffer;
    u32 debug_sphere_instance_data_alloc_pos;
    //                                          XYZR, RGBA
#define RENDERER_OPENGL_BYTES_PER_DEBUG_SPHERE (sizeof(f32) * 8)
    GLubyte debug_sphere_instance_data[RENDERER_MAX_TEXTS * RENDERER_OPENGL_BYTES_PER_TEXT];
    
    // NOTE(rjf): Request data
    u32 request_count;
    RendererRequest requests[RENDERER_MAX_REQUESTS];
    
    // NOTE(rjf): Light data
    struct
    {
        v3 position;
        v3 color;
        f32 radius;
        f32 intensity;
    }
    lights[RENDERER_MAX_LIGHT_COUNT];
    u32 light_count;
    
    // NOTE(rjf): Clipping stack
    u32 clip_stack_size;
    v4 clip_stack[RENDERER_CLIP_STACK_MAX];
    v4 current_clip;
    
    // NOTE(rjf): Game-specific rendering stuff
    RendererOpenGLFBO *active_fbo;
    RendererOpenGLFBO main_fbo;
    RendererOpenGLFBO main_2d_fbo;
    RendererOpenGLFBO world_fbo;
    RendererOpenGLFBO screen_size_scratch_fbo;
    RendererOpenGLFBO shadow_map_fbo;
    
    // NOTE(rjf): Shaders
    GLuint shaders[OPENGL_SHADER_MAX];
};

#define GLProc(type, name) PFNGL##type##PROC gl##name;
#include "opengl_procedure_list.h"