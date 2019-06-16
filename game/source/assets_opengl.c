internal Texture
TextureInitFromData(void *data, u32 len)
{
    Texture t;
    int w, h;
    u8 *tex_data = stbi_load_from_memory((unsigned char *)data, (int)len,
                                         &w, &h, 0,
                                         STBI_rgb_alpha);
    t.w = (i16)w;
    t.h = (i16)h;
    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.w, t.h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, tex_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(tex_data);
    t.loaded = 1;
    return t;
}

internal Texture
TextureLoad(char *name)
{
    Texture texture = {0};
    char filename[256] = {0};
    snprintf(filename, 256, "%s.png", name);
    void *png_data = 0;
    u32 png_data_len = 0;
    platform->LoadEntireFile(filename, &png_data, &png_data_len);
    if(png_data && png_data_len)
    {
        texture = TextureInitFromData(png_data, png_data_len);
        platform->FreeFileData(png_data);
    }
    return texture;
}

internal void
TextureCleanUp(Texture *t)
{
    if(t->loaded)
    {
        glDeleteTextures(1, &t->id);
    }
    t->id = 0;
    t->loaded = 0;
}

internal Font
FontInitFromData(void *image, u32 image_len, void *font, u32 font_len)
{
    Font f = {0};
    f.texture = TextureInitFromData(image, image_len);
    enum
    {
        READ_NONE = 0,
        READ_NAME,
        READ_VALUE,
        MAX_READ
    };
    char *data = (char *)font,
    name[32] = {0},
    value[32] = {0};
    i8 read_char = 0,
    read_state = 0,
    commented = 0;
    u32 read_pos = 0;
    for(u32 i = 0; i < font_len;)
    {
        if(data[i] == '#')
        {
            commented = 1;
        }
        if(commented)
        {
            if(data[i] == '\n')
            {
                commented = 0;
            }
            i++;
        }
        else
        {
            switch(read_state)
            {
                case READ_NONE:
                {
                    if(CharIsAlpha(data[i]))
                    {
                        read_state = READ_NAME;
                        read_pos = i;
                    }
                    else
                    {
                        i++;
                    }
                    break;
                }
                case READ_NAME:
                {
                    if(data[i] == '=')
                    {
                        i16 name_len =
                            (i16)(i + 1 - read_pos < 32 ? i + 1 - read_pos : 31);
                        strncpy(name, data + read_pos, name_len);
                        name[name_len - 1] = '\0';
                        read_state = READ_VALUE;
                        read_pos = ++i;
                    }
                    else
                    {
                        i++;
                    }
                    break;
                }
                case READ_VALUE:
                {
                    if(CharIsSpace((int)data[i]) || i == font_len - 1)
                    {
                        i16 value_len =
                            (i16)(i + 1 - read_pos < 32 ? i + 1 - read_pos : 31);
                        strncpy(value, data + read_pos, value_len);
                        value[value_len - 1] = '\0';
                        if(CStringMatchCaseSensitive(name, "char id"))
                        {
                            read_char = (i8)(GetFirstI32FromCString(value) - 32);
                            if(read_char < 0 || read_char >= 95)
                            {
                                read_char = 0;
                            }
                        }
                        else if(CStringMatchCaseSensitive(name, "x"))
                        {
                            f.char_x[read_char] = (i16)GetFirstI32FromCString(value);
                        }
                        else if(CStringMatchCaseSensitive(name, "y"))
                        {
                            f.char_y[read_char] = (i16)GetFirstI32FromCString(value);
                        }
                        else if(CStringMatchCaseSensitive(name, "width"))
                        {
                            f.char_w[read_char] = (i16)GetFirstI32FromCString(value);
                        }
                        else if(CStringMatchCaseSensitive(name, "height"))
                        {
                            f.char_h[read_char] = (i16)GetFirstI32FromCString(value);
                        }
                        else if(CStringMatchCaseSensitive(name, "xoffset"))
                        {
                            f.char_x_offset[read_char] = (i16)GetFirstI32FromCString(value);
                        }
                        else if(CStringMatchCaseSensitive(name, "yoffset"))
                        {
                            f.char_y_offset[read_char] = (i16)GetFirstI32FromCString(value);
                        }
                        else if(CStringMatchCaseSensitive(name, "xadvance"))
                        {
                            f.char_x_advance[read_char] = (i16)GetFirstI32FromCString(value) - 16;
                        }
                        else if(CStringMatchCaseSensitive(name, "size"))
                        {
                            f.size = (i16)GetFirstI32FromCString(value);
                        }
                        else if(CStringMatchCaseSensitive(name, "lineHeight"))
                        {
                            f.line_height = (i16)GetFirstI32FromCString(value);
                        }
                        read_state = READ_NONE;
                    }
                    else
                    {
                        i++;
                    }
                    break;
                }
                default:
                {
                    i++;
                    break;
                }
            }
        }
    }
    return f;
}

internal Font
FontLoad(char *name)
{
    Font font = {0};
    char filename[256] = {0};
    snprintf(filename, 256, "%s.png", name);
    void *png_data = 0;
    u32 png_data_len = 0;
    platform->LoadEntireFile(filename, &png_data, &png_data_len);
    if(png_data && png_data_len)
    {
        void *fnt_data = 0;
        u32 fnt_data_len = 0;
        snprintf(filename, 256, "%s.fnt", name);
        platform->LoadEntireFile(filename, &fnt_data, &fnt_data_len);
        if(fnt_data)
        {
            font = FontInitFromData(png_data, png_data_len,
                                    fnt_data, fnt_data_len);
            platform->FreeFileData(fnt_data);
        }
        platform->FreeFileData(png_data);
    }
    return font;
}

internal void
FontCleanUp(Font *f)
{
    if(f->texture.id)
    {
        TextureCleanUp(&f->texture);
    }
}

internal b32
TextureIsLoaded(Texture *texture)
{
    return texture->loaded;
}

internal b32
FontIsLoaded(Font *font)
{
    return font->texture.loaded;
}

internal f32
FontGetTextWidth(Font *font, char *text)
{
    f32 text_width = 0.f;
    for(u32 i = 0; text[i]; ++i)
    {
        if(text[i] >= 32 && text[i] < 127)
        {
            text_width += (f32)font->char_x_advance[text[i] - 32];
        }
    }
    return text_width;
}

internal f32
FontGetLineHeight(Font *font)
{
    return (f32)font->line_height;
}

internal Model
ModelInitFromData(f32 *data, u32 number_of_floats, i32 flags)
{
    Model model = {0};
    
    model.flags = flags;
    
    if(!flags)
    {
        goto end_init;
    }
    
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);
    {
        glGenBuffers(1, &model.vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, model.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, number_of_floats*sizeof(f32), data, GL_STATIC_DRAW);
        
        u32 floats_per_vertex = 0;
        if(flags & MODEL_FLAG_POSITION)
        {
            floats_per_vertex += 3;
        }
        if(flags & MODEL_FLAG_UV)
        {
            floats_per_vertex += 2;
        }
        if(flags & MODEL_FLAG_NORMAL)
        {
            floats_per_vertex += 3;
        }
        
        model.vertex_count = number_of_floats / floats_per_vertex;
        
        u32 offset = 0;
        if(flags & MODEL_FLAG_POSITION)
        {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, floats_per_vertex*sizeof(f32),
                                  (void *)(sizeof(f32)*offset));
            offset += 3;
        }
        if(flags & MODEL_FLAG_UV)
        {
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, floats_per_vertex*sizeof(f32),
                                  (void *)(sizeof(f32)*offset));
            offset += 2;
        }
        if(flags & MODEL_FLAG_NORMAL)
        {
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, floats_per_vertex*sizeof(f32),
                                  (void *)(sizeof(f32)*offset));
            offset += 3;
        }
        
    }
    glBindVertexArray(0);
    
    end_init:;
    return model;
}

internal void
ModelCleanUp(Model *model)
{
    glDeleteBuffers(1, &model->vertex_buffer);
    glDeleteVertexArrays(1, &model->vao);
}