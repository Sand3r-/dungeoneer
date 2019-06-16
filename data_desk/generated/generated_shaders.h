typedef enum OpenGLShaderType OpenGLShaderType;
enum OpenGLShaderType
{
OPENGL_SHADER_debug_sphere,
OPENGL_SHADER_fbo,
OPENGL_SHADER_filled_rect,
OPENGL_SHADER_fxaa,
OPENGL_SHADER_gaussian_blur,
OPENGL_SHADER_line,
OPENGL_SHADER_model,
OPENGL_SHADER_model_depth,
OPENGL_SHADER_text,
OPENGL_SHADER_texture,
OPENGL_SHADER_texture_3d,
OPENGL_SHADER_texture_3d_depth,
OPENGL_SHADER_world,
OPENGL_SHADER_MAX,
};

#define OPENGL_SHADER_INPUT_MAX 16
#define OPENGL_SHADER_OUTPUT_MAX 16
global struct
{
char *name;
OpenGLShaderInput inputs[OPENGL_SHADER_INPUT_MAX];
u32 input_count;
OpenGLShaderOutput outputs[OPENGL_SHADER_OUTPUT_MAX];
u32 output_count;
char *vert;
char *frag;
}
global_opengl_shaders[] = {
{
"Debug Sphere",
{
{ 2, "vert_sphere", },
{ 1, "vert_color", },
{ 0, "vert_position", },
},
3,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"in vec3 vert_position;\n"
"in vec4 vert_color;\n"
"in vec4 vert_sphere;\n"
"out vec4 frag_color;\n"
"\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position = vert_position;\n"
"    vec4 color = vert_color;\n"
"    vec4 sphere = vert_sphere;\n"
"    vec4 world_space = vec4(sphere.xyz + position*sphere.w, 1);\n"
"    vec4 clip_space = view_projection * world_space;\n"
"    gl_Position = clip_space;\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"\n"
"in vec4 frag_color;\n"
"out vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"    color = frag_color;\n"
"}\n"
"",
},
{
"FBO",
{
0},
0,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"out vec2 frag_uv;\n"
"out vec2 frag_position;\n"
"uniform vec2 render_size;\n"
"uniform vec4 destination;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 vertices[] = vec2[](\n"
"        vec2(0, 0),\n"
"        vec2(0, 1),\n"
"        vec2(1, 0),\n"
"        vec2(1, 1)\n"
"        );\n"
"    \n"
"    vec2 vert_position = vertices[gl_VertexID];\n"
"    \n"
"    vec4 screen_position = vec4(vert_position, 0, 1);\n"
"    screen_position.xy *= destination.zw;\n"
"    screen_position.xy += destination.xy;\n"
"    frag_position = screen_position.xy;\n"
"    screen_position.xy /= render_size;\n"
"    gl_Position = screen_position;\n"
"    frag_uv = vert_position.xy;\n"
"}\n"
"\n"
"",
"#version 330 core\n"
"\n"
"\n"
"in vec2 frag_uv;\n"
"in vec2 frag_position;\n"
"out vec4 color;\n"
"uniform vec2 uv_offset;\n"
"uniform vec2 uv_range;\n"
"uniform vec4 destination;\n"
"uniform vec2 scale;\n"
"uniform float opacity;\n"
"uniform sampler2D tex;\n"
"uniform vec2 tex_resolution;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 pixel = (uv_offset + (frag_uv * uv_range));\n"
"    vec2 sample_uv = floor(pixel) + vec2(0.5, 0.5);\n"
"    \n"
"    sample_uv.x += 1.0 - clamp((1.0 - fract(pixel.x)) * scale.x, 0.0, 1.0);\n"
"    sample_uv.y += 1.0 - clamp((1.0 - fract(pixel.y)) * scale.y, 0.0, 1.0);\n"
"    \n"
"    color = texture(tex, sample_uv / tex_resolution);\n"
"    color.xyz /= color.a;\n"
"    if(color.a > 0)\n"
"    {\n"
"        color *= opacity;\n"
"    }\n"
"    else\n"
"    {\n"
"        discard;\n"
"    }\n"
"}\n"
"\n"
"",
},
{
"Filled Rect Batch",
{
{ 7, "vert_color11", },
{ 6, "vert_color10", },
{ 5, "vert_color01", },
{ 4, "vert_color00", },
{ 3, "vert_11", },
{ 2, "vert_10", },
{ 1, "vert_01", },
{ 0, "vert_00", },
},
8,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"in vec3 vert_00;\n"
"in vec3 vert_01;\n"
"in vec3 vert_10;\n"
"in vec3 vert_11;\n"
"in vec4 vert_color00;\n"
"in vec4 vert_color01;\n"
"in vec4 vert_color10;\n"
"in vec4 vert_color11;\n"
"out vec4 rect_color;\n"
"\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 vertices[] = vec3[](vert_00, vert_01, vert_10, vert_11);\n"
"    vec4 world_space = vec4(vertices[gl_VertexID], 1);\n"
"    vec4 clip_space = view_projection * world_space;\n"
"    gl_Position = clip_space;\n"
"    vec4 colors[] = vec4[](vert_color00, vert_color01, vert_color10, vert_color11);\n"
"    rect_color = colors[gl_VertexID];\n"
"}\n"
"\n"
"",
"#version 330 core\n"
"\n"
"\n"
"in vec4 rect_color;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    color = rect_color;\n"
"}\n"
"\n"
"",
},
{
"FXAA",
{
0},
0,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"out vec2 v_rgbNW;\n"
"out vec2 v_rgbNE;\n"
"out vec2 v_rgbSW;\n"
"out vec2 v_rgbSE;\n"
"out vec2 v_rgbM;\n"
"out vec2 frag_uv;\n"
"\n"
"uniform vec2 color_tex_resolution;\n"
"\n"
"void GetTexCoords(vec2 fragCoord, vec2 resolution,\n"
"                  out vec2 v_rgbNW, out vec2 v_rgbNE,\n"
"                  out vec2 v_rgbSW, out vec2 v_rgbSE,\n"
"                  out vec2 v_rgbM)\n"
"{\n"
"	vec2 inverseVP = 1.0 / resolution.xy;\n"
"	v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;\n"
"	v_rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;\n"
"	v_rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;\n"
"	v_rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;\n"
"	v_rgbM = vec2(fragCoord * inverseVP);\n"
"    \n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec4 vertices[] = vec4[](vec4(0, 0, 0, 1),\n"
"                             vec4(0, 1, 0, 1),\n"
"                             vec4(1, 0, 0, 1),\n"
"                             vec4(1, 1, 0, 1));\n"
"    frag_uv = vertices[gl_VertexID].xy;\n"
"    vec2 fragCoord = frag_uv * color_tex_resolution;\n"
"    GetTexCoords(fragCoord, color_tex_resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);\n"
"    gl_Position = (vertices[gl_VertexID] * 2 - 1);\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"\n"
"in vec2 v_rgbNW;\n"
"in vec2 v_rgbNE;\n"
"in vec2 v_rgbSW;\n"
"in vec2 v_rgbSE;\n"
"in vec2 v_rgbM;\n"
"in vec2 frag_uv;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform vec2 color_tex_resolution;\n"
"uniform sampler2D color_tex;\n"
"\n"
"#define FXAA_REDUCE_MIN (1.0 / 128.0)\n"
"#define FXAA_REDUCE_MUL (1.0 / 8.0)\n"
"#define FXAA_SPAN_MAX   4.0\n"
"\n"
"vec4\n"
"FXAA(sampler2D tex, vec2 frag_coord, vec2 resolution,\n"
"     vec2 v_rgb_nw, vec2 v_rgb_ne, vec2 v_rgb_sw, vec2 v_rgb_se,\n"
"     vec2 v_rgbM)\n"
"{\n"
"    vec4 color;\n"
"    mediump vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);\n"
"    vec3 rgb_nw = texture2D(tex, v_rgb_nw).xyz;\n"
"    vec3 rgb_ne = texture2D(tex, v_rgb_ne).xyz;\n"
"    vec3 rgb_sw = texture2D(tex, v_rgb_sw).xyz;\n"
"    vec3 rgb_se = texture2D(tex, v_rgb_se).xyz;\n"
"    vec4 texColor = texture2D(tex, v_rgbM);\n"
"    vec3 rgbM  = texColor.xyz;\n"
"    vec3 luma = vec3(0.299, 0.587, 0.114);\n"
"    float lumaNW = dot(rgb_nw, luma);\n"
"    float lumaNE = dot(rgb_ne, luma);\n"
"    float lumaSW = dot(rgb_sw, luma);\n"
"    float lumaSE = dot(rgb_se, luma);\n"
"    float lumaM  = dot(rgbM,  luma);\n"
"    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));\n"
"    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));\n"
"    \n"
"    mediump vec2 dir;\n"
"    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));\n"
"    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));\n"
"    \n"
"    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *\n"
"                          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);\n"
"    \n"
"    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);\n"
"    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),\n"
"              max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),\n"
"                  dir * rcpDirMin)) * inverseVP;\n"
"    \n"
"    vec3 rgbA = 0.5 * (\n"
"        texture2D(tex, frag_coord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +\n"
"        texture2D(tex, frag_coord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);\n"
"    vec3 rgbB = rgbA * 0.5 + 0.25 * (\n"
"        texture2D(tex, frag_coord * inverseVP + dir * -0.5).xyz +\n"
"        texture2D(tex, frag_coord * inverseVP + dir * 0.5).xyz);\n"
"    \n"
"    float lumaB = dot(rgbB, luma);\n"
"    if ((lumaB < lumaMin) || (lumaB > lumaMax))\n"
"        color = vec4(rgbA, texColor.a);\n"
"    else\n"
"        color = vec4(rgbB, texColor.a);\n"
"    return color;\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    vec2 fragCoord = frag_uv * color_tex_resolution; \n"
"    color = FXAA(color_tex, fragCoord, color_tex_resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);\n"
"}\n"
"",
},
{
"Gaussian Blur",
{
{ 0, "vert_position", },
},
1,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"out vec2 frag_uv;\n"
"uniform vec4 destination;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 vertices[] = vec4[](vec4(0, 0, 0, 1),\n"
"                             vec4(0, 1, 0, 1),\n"
"                             vec4(1, 0, 0, 1),\n"
"                             vec4(1, 1, 0, 1));\n"
"    frag_uv = vertices[gl_VertexID].xy;\n"
"    gl_Position = vertices[gl_VertexID] * 2 - 1;\n"
"    frag_uv.y = 1 - frag_uv.y;\n"
"}\n"
"\n"
"",
"#version 330 core\n"
"\n"
"in vec2 frag_uv;\n"
"out vec4 color;\n"
"uniform sampler2D tex;\n"
"uniform vec2 tex_resolution;\n"
"uniform int radius;\n"
"uniform vec4 kernel[32];\n"
"uniform int vertical;\n"
"uniform vec4 clip;\n"
"\n"
"void main()\n"
"{\n"
"    color = vec4(0, 0, 0, 0);\n"
"    \n"
"    if(gl_FragCoord.x >= clip.x && gl_FragCoord.x <= clip.x + clip.z &&\n"
"       gl_FragCoord.y >= clip.y && gl_FragCoord.y <= clip.y + clip.w)\n"
"    {\n"
"        \n"
"        int first_kernel_index = (16 - radius/4);\n"
"        \n"
"        for(int i = 0; i < 2*radius/4; ++i)\n"
"        {\n"
"            if(vertical != 0)\n"
"            {\n"
"                color += texture(tex, frag_uv + vec2(0, -radius + i*4 + 0) / tex_resolution) * kernel[first_kernel_index + i].x;\n"
"                color += texture(tex, frag_uv + vec2(0, -radius + i*4 + 1) / tex_resolution) * kernel[first_kernel_index + i].y;\n"
"                color += texture(tex, frag_uv + vec2(0, -radius + i*4 + 2) / tex_resolution) * kernel[first_kernel_index + i].z;\n"
"                color += texture(tex, frag_uv + vec2(0, -radius + i*4 + 3) / tex_resolution) * kernel[first_kernel_index + i].w;\n"
"            }\n"
"            else\n"
"            {\n"
"                color += texture(tex, frag_uv + vec2(-radius + i*4 + 0, 0) / tex_resolution) * kernel[first_kernel_index + i].x;\n"
"                color += texture(tex, frag_uv + vec2(-radius + i*4 + 1, 0) / tex_resolution) * kernel[first_kernel_index + i].y;\n"
"                color += texture(tex, frag_uv + vec2(-radius + i*4 + 2, 0) / tex_resolution) * kernel[first_kernel_index + i].z;\n"
"                color += texture(tex, frag_uv + vec2(-radius + i*4 + 3, 0) / tex_resolution) * kernel[first_kernel_index + i].w;\n"
"            }\n"
"        }\n"
"    }\n"
"}\n"
"\n"
"",
},
{
"Line Batch",
{
{ 2, "vert_color_data", },
{ 1, "vert_2", },
{ 0, "vert_1", },
},
3,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"in vec3 vert_1;\n"
"in vec3 vert_2;\n"
"in vec4 vert_color;\n"
"out vec4 frag_color_data;\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 vertices[] = vec3[](vert_1, vert_2);\n"
"    frag_color_data = vert_color;\n"
"    \n"
"    vec4 world_space = vec4(vertices[gl_VertexID], 1);\n"
"    vec4 clip_space = view_projection * world_space;\n"
"    gl_Position = clip_space;\n"
"    \n"
"}\n"
"",
"#version 330 core\n"
"\n"
"in vec4 frag_color_data;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"    color = frag_color_data;\n"
"}\n"
"",
},
{
"Model",
{
{ 2, "vert_normal", },
{ 1, "vert_uv", },
{ 0, "vert_position", },
},
3,
{
{ 1, "normal", },
{ 0, "color", },
},
2,
"#version 330 core\n"
"\n"
"\n"
"in vec3 vert_position;\n"
"in vec2 vert_uv;\n"
"in vec3 vert_normal;\n"
"out vec2 frag_uv;\n"
"out vec3 frag_normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position = vert_position;\n"
"    vec2 uv = vert_uv;\n"
"    vec3 normal = vert_normal;\n"
"    \n"
"    vec4 world_space = vec4(position, 1);\n"
"    vec4 clip_space = view_projection * model * world_space;\n"
"    gl_Position = clip_space;\n"
"    \n"
"    frag_uv = uv;\n"
"    frag_normal = (model * vec4(normal, 0)).xyz;\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"\n"
"in vec2 frag_uv;\n"
"in vec3 frag_normal;\n"
"out vec4 color;\n"
"out vec4 normal;\n"
"uniform sampler2D tex;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 texture_sample = texture(tex, frag_uv);\n"
"    color = texture_sample;\n"
"    normal = vec4(frag_normal, 1);\n"
"}\n"
"",
},
{
"Model (Depth Only)",
{
{ 2, "vert_normal", },
{ 1, "vert_uv", },
{ 0, "vert_position", },
},
3,
{
0},
0,
"#version 330 core\n"
"\n"
"\n"
"in vec3 vert_position;\n"
"in vec2 vert_uv;\n"
"in vec3 vert_normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 position = vert_position;\n"
"    vec2 uv = vert_uv;\n"
"    vec3 normal = vert_normal;\n"
"    vec4 world_space = vec4(position, 1);\n"
"    vec4 clip_space = view_projection * model * world_space;\n"
"    gl_Position = clip_space;\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"void main()\n"
"{}\n"
"",
},
{
"Text",
{
{ 7, "vert_softness", },
{ 6, "vert_boldness", },
{ 5, "vert_color", },
{ 4, "vert_position_4", },
{ 3, "vert_position_3", },
{ 2, "vert_position_2", },
{ 1, "vert_position_1", },
{ 0, "vert_source", },
},
8,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"in vec4 vert_source;\n"
"in vec3 vert_position_1;\n"
"in vec3 vert_position_2;\n"
"in vec3 vert_position_3;\n"
"in vec3 vert_position_4;\n"
"in vec4 vert_color;\n"
"in float vert_boldness;\n"
"in float vert_softness;\n"
"\n"
"out vec2 frag_uv;\n"
"out vec4 frag_color;\n"
"out float frag_boldness;\n"
"out float frag_softness;\n"
"\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 source = vert_source;\n"
"    vec4 vertices[] = vec4[](vec4(vert_position_1, 1),\n"
"                             vec4(vert_position_2, 1),\n"
"                             vec4(vert_position_3, 1),\n"
"                             vec4(vert_position_4, 1));\n"
"    vec4 color = vert_color;\n"
"    float boldness = vert_boldness;\n"
"    float softness = vert_softness;\n"
"    \n"
"    vec4 world_space = vertices[gl_VertexID];\n"
"    vec4 clip_space = view_projection * world_space;\n"
"    gl_Position = clip_space;\n"
"    \n"
"    vec2 uvs[] = vec2[](vec2(0, 0),\n"
"                        vec2(0, 1),\n"
"                        vec2(1, 0),\n"
"                        vec2(1, 1));\n"
"    \n"
"    frag_uv = vert_source.xy + uvs[gl_VertexID]*vert_source.zw;\n"
"    frag_color = color;\n"
"    frag_boldness = boldness;\n"
"    frag_softness = softness;\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"\n"
"in vec2 frag_uv;\n"
"in vec4 frag_color;\n"
"in float frag_boldness;\n"
"in float frag_softness;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D tex;\n"
"uniform vec2 tex_resolution;\n"
"\n"
"void main()\n"
"{\n"
"    vec2 uv = frag_uv;\n"
"    vec4 text_color = frag_color;\n"
"    float boldness = frag_boldness;\n"
"    float softness = frag_softness;\n"
"    \n"
"    vec2 resolution = tex_resolution;\n"
"    vec2 pixel = uv;\n"
"    vec2 sample_uv = floor(pixel) + vec2(0.5, 0.5);\n"
"    sample_uv.x += 1.0 - clamp((1.0 - fract(pixel.x)), 0.0, 1.0);\n"
"    sample_uv.y += 1.0 - clamp((1.0 - fract(pixel.y)), 0.0, 1.0);\n"
"    float distance = texture(tex, sample_uv / resolution).a;\n"
"    float smooth_step = smoothstep(1.0 - boldness, (1.0 - boldness) + softness, distance);\n"
"    color = text_color * smooth_step;\n"
"    color.xyz /= color.a;\n"
"    if(color.a < 0.02)\n"
"    {\n"
"        discard;\n"
"    }\n"
"}\n"
"\n"
"",
},
{
"Texture Batch (2D)",
{
{ 5, "vert_color", },
{ 4, "vert_11", },
{ 3, "vert_10", },
{ 2, "vert_01", },
{ 1, "vert_00", },
{ 0, "vert_source", },
},
6,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"in vec4 vert_source;\n"
"in vec3 vert_00;\n"
"in vec3 vert_01;\n"
"in vec3 vert_10;\n"
"in vec3 vert_11;\n"
"in vec4 vert_color;\n"
"out vec2 frag_uv;\n"
"out vec4 frag_source;\n"
"out vec2 frag_scale;\n"
"out vec4 frag_tint;\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 source = vert_source;\n"
"    vec3 vertices[] = vec3[](vert_00, vert_01, vert_10, vert_11);\n"
"    vec4 tint = vert_color;\n"
"    vec4 world_space = vec4(vertices[gl_VertexID], 1);\n"
"    vec4 clip_space = view_projection * world_space;\n"
"    \n"
"    vec2 uvs[] = vec2[](vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1));\n"
"    frag_uv = uvs[gl_VertexID];\n"
"    frag_source = source;\n"
"    frag_scale = vec2(4, 4);\n"
"    frag_tint = tint;\n"
"    \n"
"    gl_Position = clip_space;\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"in vec2 frag_uv;\n"
"in vec4 frag_source;\n"
"in vec2 frag_scale;\n"
"in vec4 frag_tint;\n"
"out vec4 color;\n"
"uniform sampler2D tex;\n"
"uniform vec2 tex_resolution;\n"
"void main()\n"
"{\n"
"    vec2 uv_offset = frag_source.xy;\n"
"    vec2 uv_range = frag_source.zw;\n"
"    vec4 tint = frag_tint;\n"
"    vec2 scale = frag_scale;\n"
"    \n"
"    vec2 pixel = (uv_offset + (frag_uv * uv_range));\n"
"    vec2 sample_uv = floor(pixel) + vec2(0.5, 0.5);\n"
"    \n"
"    sample_uv.x += 1.0 - clamp((1.0 - fract(pixel.x)) * abs(scale.x), 0.0, 1.0);\n"
"    sample_uv.y += 1.0 - clamp((1.0 - fract(pixel.y)) * abs(scale.y), 0.0, 1.0);\n"
"    \n"
"    color = texture(tex, sample_uv / tex_resolution);\n"
"    \n"
"    if(color.a > 0)\n"
"    {\n"
"        color.xyz /= color.a;\n"
"        color *= tint;\n"
"    }\n"
"    else\n"
"    {\n"
"        discard;\n"
"    }\n"
"}\n"
"",
},
{
"Texture Batch (3D)",
{
{ 5, "vert_color", },
{ 4, "vert_11", },
{ 3, "vert_10", },
{ 2, "vert_01", },
{ 1, "vert_00", },
{ 0, "vert_source", },
},
6,
{
{ 1, "normal", },
{ 0, "color", },
},
2,
"#version 330 core\n"
"\n"
"\n"
"in vec4 vert_source;\n"
"in vec3 vert_00;\n"
"in vec3 vert_01;\n"
"in vec3 vert_10;\n"
"in vec3 vert_11;\n"
"in vec4 vert_color;\n"
"out vec2 frag_uv;\n"
"out vec4 frag_source;\n"
"out vec2 frag_scale;\n"
"out vec4 frag_tint;\n"
"out vec3 frag_normal;\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 source = vert_source;\n"
"    vec3 vertices[] = vec3[](vert_00, vert_01, vert_10, vert_11);\n"
"    vec4 tint = vert_color;\n"
"    vec4 world_space = vec4(vertices[gl_VertexID], 1);\n"
"    vec4 clip_space = view_projection * world_space;\n"
"    \n"
"    vec2 uvs[] = vec2[](vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1));\n"
"    frag_uv = uvs[gl_VertexID];\n"
"    frag_source = source;\n"
"    frag_scale = vec2(4, 4);\n"
"    frag_tint = tint;\n"
"    \n"
"    gl_Position = clip_space;\n"
"    \n"
"    frag_normal = normalize(cross(vert_01 - vert_00, vert_10 - vert_00));\n"
"    if(frag_normal.y < 0)\n"
"    {\n"
"        frag_normal = -frag_normal;\n"
"    }\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"in vec2 frag_uv;\n"
"in vec4 frag_source;\n"
"in vec2 frag_scale;\n"
"in vec4 frag_tint;\n"
"in vec3 frag_normal;\n"
"out vec4 color;\n"
"out vec4 normal;\n"
"uniform sampler2D tex;\n"
"uniform vec2 tex_resolution;\n"
"void main()\n"
"{\n"
"    vec2 uv_offset = frag_source.xy;\n"
"    vec2 uv_range = frag_source.zw;\n"
"    vec4 tint = frag_tint;\n"
"    vec2 scale = frag_scale;\n"
"    \n"
"    vec2 pixel = (uv_offset + (frag_uv * uv_range));\n"
"    vec2 sample_uv = floor(pixel) + vec2(0.5, 0.5);\n"
"    \n"
"    sample_uv.x += 1.0 - clamp((1.0 - fract(pixel.x)) * abs(scale.x), 0.0, 1.0);\n"
"    sample_uv.y += 1.0 - clamp((1.0 - fract(pixel.y)) * abs(scale.y), 0.0, 1.0);\n"
"    \n"
"    color = texture(tex, sample_uv / tex_resolution);\n"
"    if(color.a > 0)\n"
"    {\n"
"        color.xyz /= color.a;\n"
"        color *= tint;\n"
"        normal = vec4(frag_normal, 1);\n"
"    }\n"
"    else\n"
"    {\n"
"        discard;\n"
"    }\n"
"    \n"
"}\n"
"",
},
{
"Texture Batch (3D, Depth Only)",
{
{ 5, "vert_color", },
{ 4, "vert_11", },
{ 3, "vert_10", },
{ 2, "vert_01", },
{ 1, "vert_00", },
{ 0, "vert_source", },
},
6,
{
0},
0,
"#version 330 core\n"
"\n"
"\n"
"in vec4 vert_source;\n"
"in vec3 vert_00;\n"
"in vec3 vert_01;\n"
"in vec3 vert_10;\n"
"in vec3 vert_11;\n"
"in vec4 vert_color;\n"
"out vec2 frag_uv;\n"
"out vec4 frag_source;\n"
"out vec2 frag_scale;\n"
"out vec4 frag_tint;\n"
"uniform mat4 view_projection;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 source = vert_source;\n"
"    vec3 vertices[] = vec3[](vert_00, vert_01, vert_10, vert_11);\n"
"    vec4 tint = vert_color;\n"
"    vec4 world_space = vec4(vertices[gl_VertexID], 1);\n"
"    vec4 clip_space = view_projection * world_space;\n"
"    \n"
"    vec2 uvs[] = vec2[](vec2(0, 0), vec2(0, 1), vec2(1, 0), vec2(1, 1));\n"
"    frag_uv = uvs[gl_VertexID];\n"
"    frag_source = source;\n"
"    frag_scale = vec2(4, 4);\n"
"    frag_tint = tint;\n"
"    \n"
"    gl_Position = clip_space;\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"in vec2 frag_uv;\n"
"in vec4 frag_source;\n"
"in vec2 frag_scale;\n"
"in vec4 frag_tint;\n"
"uniform sampler2D tex;\n"
"uniform vec2 tex_resolution;\n"
"void main()\n"
"{\n"
"    vec2 uv_offset = frag_source.xy;\n"
"    vec2 uv_range = frag_source.zw;\n"
"    vec4 tint = frag_tint;\n"
"    vec2 scale = frag_scale;\n"
"    \n"
"    vec2 pixel = (uv_offset + (frag_uv * uv_range));\n"
"    vec2 sample_uv = floor(pixel) + vec2(0.5, 0.5);\n"
"    \n"
"    sample_uv.x += 1.0 - clamp((1.0 - fract(pixel.x)) * abs(scale.x), 0.0, 1.0);\n"
"    sample_uv.y += 1.0 - clamp((1.0 - fract(pixel.y)) * abs(scale.y), 0.0, 1.0);\n"
"    \n"
"    vec4 color = texture(tex, sample_uv / tex_resolution);\n"
"    if(color.a <= 0)\n"
"    {\n"
"        discard;\n"
"    }\n"
"}\n"
"",
},
{
"World",
{
0},
0,
{
{ 0, "color", },
},
1,
"#version 330 core\n"
"\n"
"\n"
"out vec2 frag_uv;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 vertices[] = vec4[](vec4(0, 0, 0, 1),\n"
"                             vec4(0, 1, 0, 1),\n"
"                             vec4(1, 0, 0, 1),\n"
"                             vec4(1, 1, 0, 1));\n"
"    frag_uv = vertices[gl_VertexID].xy;\n"
"    gl_Position = (vertices[gl_VertexID] * 2 - 1);\n"
"}\n"
"",
"#version 330 core\n"
"\n"
"\n"
"in vec2 frag_uv;\n"
"out vec4 color;\n"
"uniform mat4 inverse_view_projection;\n"
"uniform mat4 shadow_view_projection;\n"
"uniform sampler2D albedo_texture;\n"
"uniform sampler2D normal_texture;\n"
"uniform sampler2D depth_texture;\n"
"uniform sampler2DShadow shadow_map_texture;\n"
"\n"
"struct Light\n"
"{\n"
"    vec3 position;\n"
"    vec3 color;\n"
"    float radius;\n"
"    float intensity;\n"
"};\n"
"uniform Light lights[16];\n"
"uniform int light_count;\n"
"\n"
"void main()\n"
"{\n"
"    vec4 albedo = texture(albedo_texture, frag_uv);\n"
"    vec4 normal = texture(normal_texture, frag_uv);\n"
"    float depth = texture(depth_texture, frag_uv).r * 2 - 1;\n"
"    \n"
"    vec4 world_space_position;\n"
"    {\n"
"        vec4 camera_clip_space = vec4(frag_uv*2 - 1, depth, 1);\n"
"        vec4 world_space = inverse_view_projection * camera_clip_space + vec4(normal.xyz, 0)*0.02;\n"
"        world_space /= world_space.w;\n"
"        world_space_position = world_space;\n"
"    }\n"
"    \n"
"    float world_space_y_modifier = 1;\n"
"    {\n"
"        world_space_y_modifier = 1 - clamp(world_space_position.y, -15, 0) / -15;\n"
"    }\n"
"    \n"
"    float depth_modifier = 1;\n"
"    {\n"
"    }\n"
"    \n"
"    vec3 point_light_modifier = vec3(0.1);\n"
"    {\n"
"        for(int i = 0; i < light_count; ++i)\n"
"        {\n"
"            vec3 fragment_to_light = lights[i].position - world_space_position.xyz;\n"
"            float distance_squared = dot(fragment_to_light, fragment_to_light);\n"
"            float radius_squared = lights[i].radius * lights[i].radius;\n"
"            if(distance_squared < radius_squared)\n"
"            {\n"
"                float factor = clamp(1 - distance_squared/radius_squared, 0, 1);\n"
"                factor *= (dot(normal.xyz, normalize(fragment_to_light)) + 1.f) / 2.f;\n"
"                factor *= lights[i].intensity;\n"
"                point_light_modifier += factor*lights[i].color;\n"
"            }\n"
"        }\n"
"    }\n"
"    \n"
"    float shadow_modifier = 1;\n"
"    {\n"
"        vec4 shadow_clip_space = shadow_view_projection * world_space_position;\n"
"        shadow_clip_space /= shadow_clip_space.w;\n"
"        \n"
"        float actual_depth = (shadow_clip_space.z+1)/2;\n"
"        vec3 shadow_sample_pos = vec3((shadow_clip_space.xy + 1)/2, actual_depth);\n"
"        float shadow_sample = (textureOffset(shadow_map_texture, shadow_sample_pos, ivec2(0, 0)) +\n"
"                               textureOffset(shadow_map_texture, shadow_sample_pos, ivec2(-1, +0)) +\n"
"                               textureOffset(shadow_map_texture, shadow_sample_pos, ivec2(+1, +0)) +\n"
"                               textureOffset(shadow_map_texture, shadow_sample_pos, ivec2(+0, -1)) +\n"
"                               textureOffset(shadow_map_texture, shadow_sample_pos, ivec2(+0, +1))) /\n"
"            5.0;\n"
"        shadow_modifier = shadow_sample/2 + 0.5;\n"
"    }\n"
"    \n"
"    float diffuse_modifier = 1;\n"
"    {\n"
"        vec3 light_vector = normalize(vec3(1, 1, 1));\n"
"        diffuse_modifier = (1+dot(normal.xyz, light_vector))/2;\n"
"    }\n"
"    \n"
"    color = albedo;\n"
"    color.rgb *= (point_light_modifier *\n"
"                  shadow_modifier *\n"
"                  diffuse_modifier *\n"
"                  world_space_y_modifier *\n"
"                  depth_modifier);\n"
"}\n"
"",
},
};

