#define RENDERER_MAX_REQUESTS 65536
#define RENDERER_CLIP_STACK_MAX 128
#define RENDERER_MAX_LIGHT_COUNT 256

#define RENDERER_FLIP_HORIZONTAL     (1<<0)
#define RENDERER_FLIP_VERTICAL       (1<<1)
#define RENDERER_ADDITIVE_BLEND      (1<<2)
#define RENDERER_TEXT_ALIGN_CENTER_X (1<<3)
#define RENDERER_TEXT_ALIGN_CENTER_Y (1<<4)
#define RENDERER_TEXT_ALIGN_RIGHT    (1<<5)
#define RENDERER_2D                  (1<<6)
#define RENDERER_NO_SHADOW           (1<<7)
#define RENDERER_NO_DEPTH_WRITE      (1<<8)

typedef enum RendererRequestType RendererRequestType;
enum RendererRequestType
{
    RENDERER_REQUEST_null,
    
#define RENDERER_MAX_LINES 16384
    RENDERER_REQUEST_line,
    
#define RENDERER_MAX_FILLED_RECTS 16384
    RENDERER_REQUEST_filled_rect,
    
#define RENDERER_MAX_TEXTURES 32768
    RENDERER_REQUEST_texture,
    
#define RENDERER_MAX_TEXTS 16384
    RENDERER_REQUEST_text,
    
#define RENDERER_MAX_MODELS 16384
    RENDERER_REQUEST_model,
    
    RENDERER_REQUEST_set_clip,
    RENDERER_REQUEST_blur_rectangle,
    RENDERER_REQUEST_view_matrix,
    RENDERER_REQUEST_projection_matrix,
    RENDERER_REQUEST_shadow_view_matrix,
    RENDERER_REQUEST_shadow_projection_matrix,
    
    RENDERER_REQUEST_debug_sphere,
    
    RENDERER_REQUEST_MAX
};

typedef struct Renderer Renderer;
typedef struct RendererRequest RendererRequest;

#define RENDERER_COMMON_DATA        \
struct                              \
{                                   \
    RendererRequest active_request; \
    i32 flags;                      \
}

#if RENDERER_BACKEND == RENDERER_OPENGL
#include "renderer_opengl.h"
#else
#error "Non-OpenGL renderer not implemented."
#endif
