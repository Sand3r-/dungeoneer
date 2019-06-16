internal UIID
UIIDInit(i32 primary, i32 secondary)
{
    UIID id = {primary, secondary};
    return id;
}

internal UIID
UIIDNull(void)
{
    return UIIDInit(-1, -1);
}

internal UIID
UIIDNonInteractable(void)
{
    return UIIDInit(-2, -2);
}

internal b32
UIIDEqual(UIID id1, UIID id2)
{
    return (id1.primary == id2.primary && id1.secondary == id2.secondary);
}

internal b32
UIIDIsNull(UIID id)
{
    return UIIDEqual(id, UIIDNull());
}

internal UIID
UIGenerateID(UI *ui, char *text)
{
    i32 primary_id = HashCString(text) % UI_WIDGET_MAX;
    i32 secondary_id = ui->widget_id_counters[primary_id]++;
    return UIIDInit(primary_id, secondary_id);
}

internal void
UIWidgetInit(UI *ui, UIWidget *widget, UIWidgetType type, UIID id,
             v4 rect, char *text, UIWindow *parent_window)
{
    widget->type = type;
    if(!UIIDEqual(widget->id, id))
    {
        widget->hot_transition = 0.f;
        widget->active_transition = 0.f;
    }
    widget->id = id;
    widget->rect = rect;
    if(text)
    {
        widget->text = MakeStringOnMemoryArena(ui->widget_arena, "%s", text);
    }
    widget->parent_window = parent_window;
    widget->clip = ui->current_auto_layout_state.clip;
    widget->text_color = ui->current_auto_layout_state.text_color;
    widget->text_scale = ui->current_auto_layout_state.text_scale;
}

internal void
UIInit(UI *ui)
{
    ui->hot = ui->active = UIIDNull();
    MemorySet(ui->windows, 0, sizeof(ui->windows));
}

internal void
UIBeginFrame(UI *ui, UIFrameData *frame, f32 render_w, f32 render_h)
{
    ui->input = frame->input;
    ui->delta_t = frame->delta_t;
    ui->widget_arena = frame->widget_arena;
    ui->main_font = frame->main_font;
    ui->editor_font = frame->editor_font;
    ui->render_w = render_w;
    ui->render_h = render_h;
    
    ui->last_frame_widget_count = ui->widget_count;
    ui->widget_count = 0;
    
    ui->current_auto_layout_state.position = v2(0, 0);
    ui->current_auto_layout_state.size = v2(256, 48);
    ui->current_auto_layout_state.text_color = v4(1, 1, 1, 1);
    ui->current_auto_layout_state.text_scale = 0.26f;
    ui->current_auto_layout_state.is_column = 0;
    ui->current_auto_layout_state.clip = v4(0, 0, render_w, render_h);
    
    ui->x_position_stack_size = 0;
    ui->y_position_stack_size = 0;
    ui->width_stack_size = 0;
    ui->height_stack_size = 0;
    ui->text_color_stack_size = 0;
    ui->group_mode_stack_size = 0;
    ui->active_dropdown_stack_size = 0;
    
    ui->allowed_window = 0;
    ui->active_window = 0;
    
    for(i32 i = 0; i < (i32)ui->window_count; ++i)
    {
        UIWindow *window = ui->windows + ui->window_order[i];
        
        if(window->stale)
        {
            window->active = 0;
            window->deleted = 1;
            u32 window_index = ui->window_order[i];
            MemoryMove(ui->window_order, ui->window_order+1, sizeof(ui->window_order[0])*(ui->window_count - i));
            --ui->window_count;
            --i;
        }
        else
        {
            
            if(window->active && (V4RectHasPoint(window->rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)) ||
                                  ui->dragging_window == window))
            {
                ui->allowed_window = window;
                
                if(ui->input->left_mouse_pressed)
                {
                    u32 window_index = ui->window_order[i];
                    MemoryMove(ui->window_order+1, ui->window_order, sizeof(ui->window_order[0])*i);
                    ui->window_order[0] = window_index;
                }
                
                window->stale = 1;
                
                break;
            }
            
            window->stale = 1;
        }
    }
    
    if(ui->input->mouse_position_used)
    {
        ui->hot = UIIDNonInteractable();
    }
    else
    {
        // NOTE(rjf): Update the hot ID. If the cursor has moved outside of the
        // hot rectangle, then we need to reset the hot ID to null.
        if(!V4RectHasPoint(ui->hot_rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)))
        {
            ui->hot = UIIDNull();
        }
    }
    
    if(AbsoluteValue(ui->input->mouse_position.x - ui->last_mouse.x) > 0.01f ||
       AbsoluteValue(ui->input->mouse_position.y - ui->last_mouse.y) > 0.01f)
    {
        ui->mouse_mode = 1;
    }
}

internal v4
UIGetColorPickerHueSelectorRect(v4 widget_rect)
{
    v4 rect = {
        widget_rect.x + widget_rect.width - widget_rect.width / 8,
        widget_rect.y,
        widget_rect.width / 8,
        widget_rect.height - widget_rect.height / 8
    };
    return rect;
}

internal v4
UIGetColorPickerSaturationValueSelectorRect(v4 widget_rect)
{
    v4 rect = {
        widget_rect.x,
        widget_rect.y,
        widget_rect.width - widget_rect.width / 8,
        widget_rect.height - widget_rect.height / 8
    };
    return rect;
}

internal v4
UIGetColorPickerExtraInfoRect(v4 widget_rect)
{
    v4 rect = {
        widget_rect.x,
        widget_rect.y + widget_rect.height - widget_rect.height / 8,
        widget_rect.width,
        widget_rect.height / 8,
    };
    return rect;
}

typedef struct UINote
{
    i32 note;
    v4 rect;
}
UINote;

internal void
UIGetNotePickerKeys(v4 widget_rect,
                    UINote *white_notes, u32 white_note_count,
                    UINote *black_notes, u32 black_note_count)
{
    v4 white_key_rect = {
        widget_rect.x,
        widget_rect.y,
        widget_rect.width / white_note_count,
        widget_rect.height,
    };
    
    i32 note_val = 0;
    
    for(u32 i = 0; i < white_note_count; ++i)
    {
        white_notes[i].note = note_val;
        if(note_val % 12 == 4 || note_val % 12 == 11)
        {
            ++note_val;
        }
        else
        {
            note_val += 2;
        }
        white_notes[i].rect = white_key_rect;
        white_notes[i].rect.x += i * white_key_rect.width;
    }
    
    v4 black_key_rect = {
        widget_rect.x + white_key_rect.width - white_key_rect.width / 4,
        widget_rect.y,
        white_key_rect.width / 2,
        widget_rect.height * (1.15f / 2.f)
    };
    
    note_val = 1;
    
    for(u32 i = 0; i < black_note_count; ++i)
    {
        black_notes[i].note = note_val;
        black_notes[i].rect = black_key_rect;
        black_key_rect.x += white_key_rect.width;
        if(note_val % 12 == 3 || note_val % 12 == 10)
        {
            black_key_rect.x += white_key_rect.width;
            ++note_val;
        }
        note_val += 2;
    }
}

internal void
UIRenderAndUpdateWindow(Renderer *renderer, UIWindow *window, Font *font)
{
    RendererPushRectangleBlur(renderer, window->rect, 0.5f);
    RendererPushFilledRect2D(renderer, v4(0, 0, 0, 0.8f), window->rect);
    
    if(window->flags & UI_WINDOW_FLAG_title_bar)
    {
        RendererPushFilledRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f),
                                 v4(window->rect.x, window->rect.y, window->rect.width, 30));
        RendererPushText2D(renderer, font, RENDERER_TEXT_ALIGN_CENTER_Y, v4(1, 1, 1, 1),
                           v2(window->rect.x + 10, window->rect.y + 20), 0.30f, window->title);
    }
    RendererPushRect2D(renderer, v4(0.6f, 0.6f, 0.6f, 0.8f), window->rect);
}

internal void
UIRenderAndUpdateWidget(UI *ui, Renderer *renderer, UIWidget *widget, Font *font, Font *editor_font)
{
    // RendererPushClipThatIsConstrainedByCurrent(renderer, widget->clip);
    
    if(widget->id.primary >= 0 && widget->id.primary < UI_WIDGET_MAX)
    {
        ui->widget_id_counters[widget->id.primary] = 0;
    }
    
    if(UIIDEqual(widget->id, ui->hot))
    {
        widget->hot_transition += (1.f - widget->hot_transition) * ui->delta_t * 16.f;
    }
    else
    {
        widget->hot_transition += (0.f - widget->hot_transition) * ui->delta_t * 16.f;
    }
    
    if(UIIDEqual(widget->id, ui->active))
    {
        widget->active_transition += (1.f - widget->active_transition) * ui->delta_t * 16.f;
    }
    else
    {
        widget->active_transition += (0.f - widget->active_transition) * ui->delta_t * 16.f;
    }
    
    f32 widget_x = widget->rect.x;
    f32 widget_y = widget->rect.y;
    f32 widget_w = widget->rect.width;
    f32 widget_h = widget->rect.height;
    
    char *widget_text = widget->text.data;
    
    switch(widget->type)
    {
        
        case UI_WIDGET_button:
        {
            f32 widget_text_scale = 0.5f +
                0.05f * widget->hot_transition -
                0.07f * widget->active_transition;
            RendererPushText2D(renderer, font, RENDERER_TEXT_ALIGN_CENTER_X | RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(1, 1, 1, 1),
                               v2(widget_x + widget_w / 2, widget_y + widget_h / 2 + 4),
                               widget_text_scale, widget_text);
            break;
        }
        
        case UI_WIDGET_toggler:
        {
            f32 widget_text_scale = 0.5f +
                0.05f * widget->hot_transition -
                0.07f * widget->active_transition;
            
            f32 text_width =
                RendererPushText2D(renderer,
                                   font,
                                   RENDERER_TEXT_ALIGN_CENTER_X |
                                   RENDERER_TEXT_ALIGN_CENTER_Y,
                                   v4(1, 1, 1, 1),
                                   v2(widget_x + widget_w / 2 - 16 * widget->toggler.toggle_transition,
                                      widget_y + widget_h / 2 + 4),
                                   widget_text_scale,
                                   widget_text);
            
            if(widget->toggler.toggled)
            {
                widget->toggler.toggle_transition += (1 - widget->toggler.toggle_transition) * 8.f * ui->delta_t;
            }
            else
            {
                widget->toggler.toggle_transition += (0 - widget->toggler.toggle_transition) * 8.f * ui->delta_t;
            }
            
#if 0
            RendererPushTexture(renderer,
                                core->content_data.ui_texture,
                                0,
                                v4(96, 64, 48, 48),
                                v4(widget_x + widget_w / 2 + text_width / 2 - 16 + 16 * widget->toggler.toggle_transition,
                                   widget_y + widget_h / 2 - 24,
                                   48, 48),
                                widget->toggler.toggle_transition);
#endif
            
            break;
        }
        
        case UI_WIDGET_slider:
        {
            f32 widget_text_scale = 0.5f;
            f32 border_brightness = 0.6f + 0.4f * widget->hot_transition;
            f32 fill_brightness = 0.2f + 0.22f * widget->hot_transition;
            
            RendererPushFilledRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.2f),
                                     v4(widget_x, widget_y + 6, widget_w, widget_h - 13));
            
            RendererPushFilledRect2D(renderer,
                                     v4(fill_brightness, fill_brightness, fill_brightness, 0.6f),
                                     v4(widget_x, widget_y + 6, widget_w * widget->slider.slider_percentage,
                                        widget_h - 13));
            
            RendererPushRect2D(renderer,
                               v4(border_brightness, border_brightness, border_brightness, 1),
                               v4(widget_x, widget_y + 6, widget_w, widget_h - 13));
            
            RendererPushText2D(renderer, font,
                               RENDERER_TEXT_ALIGN_CENTER_X |
                               RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(1, 1, 1, 1),
                               v2(widget_x + widget_w / 2,
                                  widget_y + widget_h / 2 + 4),
                               widget_text_scale,
                               MakeStringOnMemoryArena(ui->widget_arena, "%s (%.2f)", widget_text, widget->slider.slider_value).data);
            
            break;
        }
        
        case UI_WIDGET_title:
        {
            RendererPushText2D(renderer,
                               font,
                               RENDERER_TEXT_ALIGN_CENTER_X |
                               RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(1, 1, 1, 1),
                               v2(widget_x + widget_w / 2,
                                  widget_y + widget_h / 2 + 4),
                               0.8f,
                               widget_text);
            break;
        }
        
        case UI_WIDGET_editor_button:
        {
            RendererPushFilledRect2D(renderer, v4(0, 0, 0, 0.8f), widget->rect);
            RendererPushFilledRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f),
                                     v4(widget_x, widget_y, widget_w, widget_h*widget->hot_transition));
            RendererPushRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f), widget->rect);
            
            RendererPushText2D(renderer,
                               editor_font,
                               RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(1, 1, 1, 1),
                               v2(widget_x + 16,
                                  widget_y + widget_h / 2 + 4 + 4 * widget->active_transition),
                               widget->text_scale,
                               widget_text);
            
            break;
        }
        
        case UI_WIDGET_editor_close_button:
        {
            RendererPushClip(renderer, v4(0, 0, ui->render_w, ui->render_h));
            RendererPushText2D(renderer,
                               editor_font,
                               RENDERER_TEXT_ALIGN_CENTER_X | RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(0.7f + 0.3f * widget->hot_transition,
                                  0.7f,
                                  0.7f,
                                  1),
                               v2(widget_x + widget_w/2,
                                  widget_y + widget_h/2 + 2.5f),
                               widget->text_scale + 0.03f * widget->hot_transition,
                               widget_text);
            RendererPopClip(renderer);
            break;
        }
        
        case UI_WIDGET_editor_image_button:
        {
            Texture *texture = widget->image_button.texture;
            v4 source = widget->image_button.source;
            v4 destination = widget->rect;
            RendererPushTexture2D(renderer, texture, source, destination,
                                  v4u(0.6f + widget->hot_transition * 0.4f));
            break;
        }
        
        case UI_WIDGET_editor_toggler:
        {
            RendererPushFilledRect2D(renderer, v4(0, 0, 0, 0.8f), widget->rect);
            RendererPushFilledRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f),
                                     v4(widget_x, widget_y, widget_w, widget_h*widget->hot_transition));
            RendererPushRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f), widget->rect);
            
            RendererPushText2D(renderer,
                               editor_font,
                               RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(1, 1, 1, 1),
                               v2(widget_x + 16,
                                  widget_y + widget_h / 2 + 4 + 4 * widget->active_transition),
                               widget->text_scale,
                               widget_text);
            
            f32 check_size = widget->rect.height - 2 * widget->rect.height / 4;
            
            if(widget->toggler.toggled)
            {
                RendererPushFilledRect2D(renderer, v4(0.7f, 0.55f, 0.2f, 0.7f),
                                         v4(widget->rect.x + widget->rect.width - check_size - check_size/2,
                                            widget->rect.y + widget->rect.height/2 - check_size/2,
                                            check_size, check_size));
            }
            
            RendererPushRect2D(renderer, v4(0.7f, 0.7f, 0.7f, 0.7f),
                               v4(widget->rect.x + widget->rect.width - check_size - check_size/2,
                                  widget->rect.y + widget->rect.height/2 - check_size/2,
                                  check_size, check_size));
            
            break;
        }
        
        case UI_WIDGET_editor_slider:
        {
            String slider_text = MakeStringOnMemoryArena(ui->widget_arena, "%s : %.2f",
                                                         widget->text.data,
                                                         widget->slider.slider_value);
            
            RendererPushFilledRect2D(renderer, v4(0, 0, 0, 0.8f), widget->rect);
            RendererPushFilledRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f),
                                     v4(widget_x, widget_y, widget_w, widget_h*widget->hot_transition));
            RendererPushRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f), widget->rect);
            
            RendererPushFilledRect2D(renderer,
                                     v4(0.7f, 0.55f, 0.2f, 0.7f),
                                     v4(widget_x, widget_y,
                                        widget_w * widget->slider.slider_percentage, widget_h));
            
            RendererPushText2D(renderer,
                               editor_font,
                               RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(1, 1, 1, 1),
                               v2(widget_x + 16,
                                  widget_y + widget_h / 2 + 4),
                               widget->text_scale,
                               slider_text.data);
            
            break;
        }
        
        case UI_WIDGET_editor_line_edit:
        {
            char *widget_edit_text = widget->line_edit.edit_text.data;
            
            RendererPushFilledRect2D(renderer, v4(0, 0, 0, 0.8f), widget->rect);
            RendererPushFilledRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f),
                                     v4(widget_x, widget_y, widget_w, widget_h*widget->hot_transition));
            RendererPushRect2D(renderer, v4(0.2f, 0.2f, 0.2f, 0.4f), widget->rect);
            
            RendererPushClipThatIsConstrainedByCurrent(renderer, widget->rect);
            if(widget->line_edit.edit_text.data[0] || UIIDEqual(ui->active, widget->id))
            {
                f32 text_width = FontGetTextWidth(editor_font, widget_edit_text) * 0.26f;
                
                f32 target_view_offset = 0.f;
                if(text_width > widget->rect.width - 32)
                {
                    target_view_offset = text_width - (widget->rect.width - 32);
                }
                widget->line_edit.view_offset += ui->delta_t * 24.f * (target_view_offset - widget->line_edit.view_offset);
                
                RendererPushText2D(renderer,
                                   editor_font,
                                   RENDERER_TEXT_ALIGN_CENTER_Y,
                                   v4(1, 1, 1, 1),
                                   v2(widget_x + 16 - widget->line_edit.view_offset,
                                      widget_y + widget_h / 2 + 4),
                                   widget->text_scale,
                                   widget_edit_text);
                
                ui->caret_x_offset += ui->delta_t * 16.f * (text_width - ui->caret_x_offset);
                
                RendererPushText2D(renderer,
                                   editor_font,
                                   RENDERER_TEXT_ALIGN_CENTER_Y,
                                   V4MultiplyF32(v4(1, 1, 1, 1), widget->active_transition),
                                   v2(widget_x + 16 + ui->caret_x_offset - widget->line_edit.view_offset,
                                      widget_y + widget_h / 2 + 4),
                                   widget->text_scale,
                                   "|");
            }
            else
            {
                RendererPushText2D(renderer,
                                   editor_font,
                                   RENDERER_TEXT_ALIGN_CENTER_Y,
                                   v4(0.5f, 0.5f, 0.5f, 0.5f),
                                   v2(widget_x + 16,
                                      widget_y + widget_h / 2 + 4 + 4 * widget->active_transition),
                                   widget->text_scale,
                                   widget_text);
            }
            
            RendererPopClip(renderer);
            
            break;
        }
        
        case UI_WIDGET_editor_color_picker:
        {
            v4 widget_rect = {widget_x, widget_y, widget_w, widget_h};
            v4 h_selector_rect = UIGetColorPickerHueSelectorRect(widget_rect);
            v4 sv_selector_rect = UIGetColorPickerSaturationValueSelectorRect(widget_rect);
            
            // NOTE(rjf): Render Saturation/Value Quad
            {
                v3 full_sv_rgb = HSVToRGB(v3(widget->color_picker.hsv.x, 1, 1));
                
                f32 r = full_sv_rgb.r;
                f32 g = full_sv_rgb.g;
                f32 b = full_sv_rgb.b;
                
                v4 color00 = {1, 1, 1, 1};
                v4 color01 = {0, 0, 0, 1};
                v4 color10 = {r, g, b, 1};
                v4 color11 = {0, 0, 0, 1};
                
                RendererPushFilledRect2DV(renderer, color00, color01, color10, color11,
                                          sv_selector_rect);
            }
            
            // NOTE(rjf): Render Hue Selection
            {
                v2 hue_ranges[] = {
                    {0.f, 60.f},
                    {60.f, 120.f},
                    {120.f, 180.f},
                    {180.f, 240.f},
                    {240.f, 300.f},
                    {300.f, 360.f},
                };
                
                for(u32 i = 0; i < ArrayCount(hue_ranges); ++i)
                {
                    hue_ranges[i].x /= 360.f;
                    hue_ranges[i].y /= 360.f;
                }
                
                f32 hue_range_rectangle_height = h_selector_rect.height / ArrayCount(hue_ranges);
                
                for(u32 i = 0; i < ArrayCount(hue_ranges); ++i)
                {
                    v3 top_rgb = HSVToRGB(v3(hue_ranges[i].x, 1, 1));
                    v3 bottom_rgb = HSVToRGB(v3(hue_ranges[i].y, 1, 1));
                    
                    v4 color00 = {top_rgb.x, top_rgb.y, top_rgb.z, 1};
                    v4 color01 = {bottom_rgb.x, bottom_rgb.y, bottom_rgb.z, 1};
                    v4 color10 = {top_rgb.x, top_rgb.y, top_rgb.z, 1};
                    v4 color11 = {bottom_rgb.x, bottom_rgb.y, bottom_rgb.z, 1};
                    
                    RendererPushFilledRect2DV(renderer, color00, color01, color10, color11,
                                              v4(h_selector_rect.x,
                                                 h_selector_rect.y + hue_range_rectangle_height * i,
                                                 h_selector_rect.width,
                                                 hue_range_rectangle_height));
                }
            }
            
            // NOTE(rjf): Render color information
            {
                v4 extra_info_rect = UIGetColorPickerExtraInfoRect(widget_rect);
                
                RendererPushFilledRect2D(renderer,
                                         v4(1, 1, 1, 1),
                                         v4(extra_info_rect.x + 1,
                                            extra_info_rect.y + 1,
                                            extra_info_rect.height - 2,
                                            extra_info_rect.height - 2));
                
                RendererPushFilledRect2D(renderer, v4(widget->color_picker.rgb.r,
                                                      widget->color_picker.rgb.g,
                                                      widget->color_picker.rgb.b,
                                                      1),
                                         v4(extra_info_rect.x + 2, extra_info_rect.y + 2,
                                            extra_info_rect.height - 4, extra_info_rect.height - 4));
                
                char r_text[16] = {0};
                char g_text[16] = {0};
                char b_text[16] = {0};
                
                snprintf(r_text, sizeof(r_text), "R: %i", (int)((u8)(widget->color_picker.rgb.r * 255.f)));
                snprintf(g_text, sizeof(g_text), "G: %i", (int)((u8)(widget->color_picker.rgb.g * 255.f)));
                snprintf(b_text, sizeof(b_text), "B: %i", (int)((u8)(widget->color_picker.rgb.b * 255.f)));
                
                RendererPushText2D(renderer, editor_font, RENDERER_TEXT_ALIGN_CENTER_Y, v4(1, 1, 1, 1),
                                   v2(extra_info_rect.x + extra_info_rect.height + 16,
                                      extra_info_rect.y + extra_info_rect.height / 2 + 4),
                                   0.25f,
                                   r_text);
                
                RendererPushText2D(renderer, editor_font, RENDERER_TEXT_ALIGN_CENTER_Y, v4(1, 1, 1, 1),
                                   v2(extra_info_rect.x + extra_info_rect.height + 80,
                                      extra_info_rect.y + extra_info_rect.height / 2 + 4),
                                   0.25f,
                                   g_text);
                
                RendererPushText2D(renderer, editor_font, RENDERER_TEXT_ALIGN_CENTER_Y, v4(1, 1, 1, 1),
                                   v2(extra_info_rect.x + extra_info_rect.height + 144,
                                      extra_info_rect.y + extra_info_rect.height / 2 + 4),
                                   0.25f,
                                   b_text);
            }
            
            RendererPushFilledRect2D(renderer,
                                     v4(1, 1, 1, 1),
                                     v4(widget_x + widget->color_picker.hsv.y * sv_selector_rect.width - 2,
                                        widget_y + (1 - widget->color_picker.hsv.z) * sv_selector_rect.height - 2,
                                        4, 4));
            
            RendererPushFilledRect2D(renderer,
                                     v4(1, 1, 1, 1),
                                     v4(h_selector_rect.x,
                                        h_selector_rect.y + widget->color_picker.hsv.x * h_selector_rect.height,
                                        h_selector_rect.width, 2));
            
            break;
        }
        
        case UI_WIDGET_editor_label:
        {
            RendererPushText2D(renderer,
                               editor_font,
                               RENDERER_TEXT_ALIGN_CENTER_Y,
                               v4(0.6f, 0.6f, 0.6f, 0.6f),
                               v2(widget_x + 4,
                                  widget_y + widget_h / 2 + 4),
                               0.26f,
                               widget_text);
            break;
        }
        
        case UI_WIDGET_editor_title:
        {
            RendererPushFilledRect2D(renderer,
                                     v4(0.1f, 0.1f, 0.1f, 0.8f),
                                     v4(widget_x, widget_y, widget_w, widget_h));
            RendererPushText2DWithBoldnessAndSoftness(renderer,
                                                      editor_font,
                                                      RENDERER_TEXT_ALIGN_CENTER_Y,
                                                      v4(1, 1, 1, 1),
                                                      v2(widget_x + 16,
                                                         widget_y + widget_h / 2 + 4),
                                                      0.35f,
                                                      0.7f, 0.2f,
                                                      widget_text);
            break;
        }
        
        case UI_WIDGET_editor_rect:
        {
            RendererPushFilledRect2D(renderer,
                                     v4(0, 0, 0, 0.8f),
                                     v4(widget_x, widget_y, widget_w, widget_h));
            break;
        }
        
        case UI_WIDGET_canvas:
        {
            widget->canvas.Render(widget->text.data, widget->rect, v2(ui->input->mouse_position.x - widget->rect.x, ui->input->mouse_position.y - widget->rect.y),
                                  widget->canvas.render_user_data);
            break;
        }
        
        default: break;
    }
    
    // RendererPopClip(renderer);
}

internal void
UIEndFrame(UI *ui, Renderer *renderer)
{
    b32 found_hot = 0;
    b32 found_active = 0;
    
    for(i32 i = ui->widget_count - 1; i >= 0; --i)
    {
        if(UIIDEqual(ui->widgets[i].id, ui->hot) &&
           (ui->allowed_window == ui->widgets[i].parent_window || ui->widgets[i].parent_window == UI_WINDOW_top))
        {
            found_hot = 1;
        }
        
        if(UIIDEqual(ui->widgets[i].id, ui->active))
        {
            found_active = 1;
        }
        
        if(ui->widgets[i].parent_window == 0)
        {
            UIRenderAndUpdateWidget(ui, renderer, ui->widgets+i, ui->main_font, ui->editor_font);
        }
    }
    
    for(i32 j = ui->window_count-1; j >= 0; --j)
    {
        UIWindow *window = ui->windows + ui->window_order[j];
        
        if(window->active)
        {
            UIRenderAndUpdateWindow(renderer, window, ui->editor_font);
            if(window->flags & UI_WINDOW_FLAG_title_bar)
            {
                RendererPushClipThatIsConstrainedByCurrent(
                    renderer,
                    v4(window->rect.x, window->rect.y + 31,
                       window->rect.width,
                       window->rect.height - 31)
                    );
            }
            else
            {
                RendererPushClipThatIsConstrainedByCurrent(renderer, window->rect);
            }
            for(i32 i = (i32)window->widget_index_end - 1; i >= (i32)window->widget_index_start; --i)
            {
                UIRenderAndUpdateWidget(ui, renderer, ui->widgets+i, ui->main_font, ui->editor_font);
            }
            RendererPopClip(renderer);
        }
    }
    
    for(i32 i = ui->widget_count - 1; i >= 0; --i)
    {
        if(ui->widgets[i].parent_window == UI_WINDOW_top)
        {
            UIRenderAndUpdateWidget(ui, renderer, ui->widgets+i, ui->main_font, ui->editor_font);
        }
    }
    
    if(!found_hot)
    {
        ui->hot = UIIDNull();
    }
    if(!found_active)
    {
        ui->active = UIIDNull();
    }
    
    ui->last_mouse = ui->input->mouse_position;
}

internal void
UIPushX(UI *ui, f32 x)
{
    HardAssert(ui->x_position_stack_size < UI_STACK_MAX);
    ui->x_position_stack[ui->x_position_stack_size++].x = ui->current_auto_layout_state.position.x;
    ui->current_auto_layout_state.position.x += x;
}

internal void
UIPopX(UI *ui)
{
    if(ui->x_position_stack_size > 0)
    {
        --ui->x_position_stack_size;
        ui->current_auto_layout_state.position.x = ui->x_position_stack[ui->x_position_stack_size].x;
    }
}

internal void
UIPushY(UI *ui, f32 y)
{
    HardAssert(ui->y_position_stack_size < UI_STACK_MAX);
    ui->y_position_stack[ui->y_position_stack_size++].y = ui->current_auto_layout_state.position.y;
    ui->current_auto_layout_state.position.y += y;
}

internal void
UIPopY(UI *ui)
{
    if(ui->y_position_stack_size > 0)
    {
        --ui->y_position_stack_size;
        ui->current_auto_layout_state.position.y = ui->y_position_stack[ui->y_position_stack_size].y;
    }
}

internal void
_UIPushWidth(UI *ui, f32 width, b32 calculate)
{
    HardAssert(ui->width_stack_size < UI_STACK_MAX);
    ui->width_stack[ui->width_stack_size].calculate_width_with_text =
        ui->current_auto_layout_state.calculate_width_with_text;
    ui->width_stack[ui->width_stack_size++].width = ui->current_auto_layout_state.size.x;
    ui->current_auto_layout_state.size.x = width;
    ui->current_auto_layout_state.calculate_width_with_text = calculate;
}

internal void
UIPushWidth(UI *ui, f32 width)
{
    _UIPushWidth(ui, width, 0);
}

internal void
UIPushAutoWidth(UI *ui)
{
    _UIPushWidth(ui, 0, 1);
}

internal void
UIPopWidth(UI *ui)
{
    if(ui->width_stack_size > 0)
    {
        --ui->width_stack_size;
        ui->current_auto_layout_state.size.x = ui->width_stack[ui->width_stack_size].width;
        ui->current_auto_layout_state.calculate_width_with_text = ui->width_stack[ui->width_stack_size].calculate_width_with_text;
    }
}

internal void
_UIPushHeight(UI *ui, f32 height, b32 calculate)
{
    HardAssert(ui->height_stack_size < UI_STACK_MAX);
    ui->height_stack[ui->height_stack_size].calculate_height_with_text =
        ui->current_auto_layout_state.calculate_height_with_text;
    ui->height_stack[ui->height_stack_size++].height = ui->current_auto_layout_state.size.y;
    ui->current_auto_layout_state.size.y = height;
    ui->current_auto_layout_state.calculate_height_with_text = calculate;
}

internal void
UIPushHeight(UI *ui, f32 height)
{
    _UIPushHeight(ui, height, 0);
}

internal void
UIPushAutoHeight(UI *ui)
{
    _UIPushHeight(ui, 0, 1);
}

internal void
UIPopHeight(UI *ui)
{
    if(ui->height_stack_size > 0)
    {
        --ui->height_stack_size;
        ui->current_auto_layout_state.size.y = ui->height_stack[ui->height_stack_size].height;
        ui->current_auto_layout_state.calculate_height_with_text = ui->height_stack[ui->height_stack_size].calculate_height_with_text;
    }
}

internal void
UIPushTextColor(UI *ui, v4 color)
{
    HardAssert(ui->text_color_stack_size < UI_STACK_MAX);
    ui->text_color_stack[ui->text_color_stack_size++].color = ui->current_auto_layout_state.text_color;
    ui->current_auto_layout_state.text_color = color;
}

internal void
UIPopTextColor(UI *ui)
{
    if(ui->text_color_stack_size > 0)
    {
        --ui->text_color_stack_size;
        ui->current_auto_layout_state.text_color = ui->text_color_stack[ui->text_color_stack_size].color;
    }
}

internal void
UIPushTextScale(UI *ui, f32 scale)
{
    HardAssert(ui->text_scale_stack_size < UI_STACK_MAX);
    ui->text_scale_stack[ui->text_scale_stack_size++].scale = ui->current_auto_layout_state.text_scale;
    ui->current_auto_layout_state.text_scale = scale;
}

internal void
UIPopTextScale(UI *ui)
{
    if(ui->text_scale_stack_size > 0)
    {
        --ui->text_scale_stack_size;
        ui->current_auto_layout_state.text_scale = ui->text_scale_stack[ui->text_color_stack_size].scale;
    }
}

internal void
UIPushGroupMode(UI *ui, b32 is_column)
{
    HardAssert(ui->group_mode_stack_size < UI_STACK_MAX);
    ui->group_mode_stack[ui->group_mode_stack_size++].is_column = ui->current_auto_layout_state.is_column;
    ui->current_auto_layout_state.is_column = is_column;
}

internal void
UIPopGroupMode(UI *ui)
{
    if(ui->group_mode_stack_size > 0)
    {
        --ui->group_mode_stack_size;
        ui->current_auto_layout_state.is_column = ui->group_mode_stack[ui->group_mode_stack_size].is_column;
    }
}

internal void
UIPushPosition(UI *ui, v2 pos)
{
    UIPushX(ui, pos.x);
    UIPushY(ui, pos.y);
}

internal void
UIPopPosition(UI *ui)
{
    UIPopX(ui);
    UIPopY(ui);
}

internal void
_UIPushSize(UI *ui, v2 size, b32 calculate_width, b32 calculate_height)
{
    _UIPushWidth(ui, size.x, calculate_width);
    _UIPushHeight(ui, size.y, calculate_height);
}

internal void
UIPushSize(UI *ui, v2 size)
{
    UIPushWidth(ui, size.x);
    UIPushHeight(ui, size.y);
}

internal void
UIPopSize(UI *ui)
{
    UIPopWidth(ui);
    UIPopHeight(ui);
}

internal void
UIPushColumn(UI *ui, v2 position, v2 size)
{
    UIPushPosition(ui, position);
    UIPushSize(ui, size);
    UIPushGroupMode(ui, 1);
}

internal void
UIPushCenteredColumn(UI *ui, v2 size, u32 number_of_widgets)
{
    v2 position = {
        ui->render_w/2 - size.x/2,
        ui->render_h/2 - size.y*number_of_widgets/2,
    };
    UIPushPosition(ui, position);
    UIPushSize(ui, size);
    UIPushGroupMode(ui, 1);
}

internal void
UIPopColumn(UI *ui)
{
    UIPopPosition(ui);
    UIPopSize(ui);
    UIPopGroupMode(ui);
}

internal void
UIPushRow(UI *ui, v2 position, v2 size)
{
    UIPushPosition(ui, position);
    UIPushSize(ui, size);
    UIPushGroupMode(ui, 0);
}

internal void
UIPushAutoRow(UI *ui, v2 position, f32 height)
{
    UIPushPosition(ui, position);
    _UIPushSize(ui, v2(0, height), 1, 0);
    UIPushGroupMode(ui, 0);
}

internal void
UIPopRow(UI *ui)
{
    UIPopPosition(ui);
    UIPopSize(ui);
    UIPopGroupMode(ui);
}

internal void
UIPushClip(UI *ui, v4 clip)
{
    HardAssert(ui->clip_stack_size < UI_STACK_MAX);
    ui->clip_stack[ui->clip_stack_size++] = ui->current_auto_layout_state.clip;
    ui->current_auto_layout_state.clip = clip;
}

internal void
UIPopClip(UI *ui)
{
    if(ui->clip_stack_size)
    {
        ui->current_auto_layout_state.clip = ui->clip_stack[--ui->clip_stack_size];
    }
}

internal void
UIGetAndUpdateAutoLayoutState(UI *ui, Font *font, char *text, v4 *rect, v4 *text_color)
{
    rect->x = ui->current_auto_layout_state.position.x;
    rect->y = ui->current_auto_layout_state.position.y;
    
    if(text && ui->current_auto_layout_state.calculate_width_with_text)
    {
        rect->z = FontGetTextWidth(font, text) * ui->current_auto_layout_state.text_scale + 30;
    }
    else
    {
        rect->z = ui->current_auto_layout_state.size.x;
    }
    
    if(text && ui->current_auto_layout_state.calculate_height_with_text)
    {
        rect->w = font->line_height * ui->current_auto_layout_state.text_scale + 30;
    }
    else
    {
        rect->w = ui->current_auto_layout_state.size.y;
    }
    
    *text_color = ui->current_auto_layout_state.text_color;
    
    if(ui->current_auto_layout_state.is_column)
    {
        ui->current_auto_layout_state.position.y += rect->height;
    }
    else
    {
        ui->current_auto_layout_state.position.x += rect->width;
    }
    
    UIWindow *window = ui->active_window;
    if(window && window != UI_WINDOW_top)
    {
        f32 window_relative_position = (rect->y - window->rect.height + 64) - (window->rect.y - window->view_offset.y);
        if(window_relative_position > window->target_view_max.y)
        {
            window->target_view_max.y = window_relative_position;
        }
    }
}

internal void
UIWidgetUpdateNonInteractable(UI *ui, v4 rect)
{
    b32 cursor_over = V4RectHasPoint(rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y));
    if(UIIDIsNull(ui->hot) && cursor_over)
    {
        ui->hot = UIIDNonInteractable();
        ui->input->mouse_position_used = 1;
    }
}

internal b32
_UIUpdateClickableWidget(UI *ui, UIID id, v4 rect, char *text, b32 editor)
{
    b32 result = 0;
    
    if(ui->allowed_window == ui->active_window || ui->active_window == UI_WINDOW_top)
    {
        b32 keyboard_mode = !ui->mouse_mode;
        
        if(keyboard_mode)
        {
#if 0
            // TODO(rjf)
            if(UIIDEqual(id, ui->hot))
            {
                if(ui->enter_pressed)
                {
                    result = 1;
                }
            }
#endif
        }
        else
        {
            b32 cursor_over =
                V4RectHasPoint(rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)) &&
                V4RectHasPoint(ui->current_auto_layout_state.clip, v2(ui->input->mouse_position.x, ui->input->mouse_position.y));
            
            if(UIIDEqual(ui->active, id))
            {
                if(!ui->input->left_mouse_down)
                {
                    result = cursor_over;
                    ui->active = UIIDNull();
                    if(!cursor_over)
                    {
                        ui->hot = UIIDNull();
                    }
                }
                ui->input->mouse_position_used = 1;
                ui->input->mouse_buttons_used = 1;
            }
            else
            {
                if(UIIDEqual(ui->hot, id))
                {
                    if(ui->input->left_mouse_down)
                    {
                        ui->active = id;
                        ui->input->mouse_position_used = 1;
                        ui->input->mouse_buttons_used = 1;
                    }
                    else if(!cursor_over)
                    {
                        ui->hot = UIIDNull();
                    }
                }
                else
                {
                    if((UIIDIsNull(ui->hot) || !ui->hot_is_on_top) && cursor_over &&
                       !ui->input->left_mouse_down)
                    {
                        ui->hot = id;
                        ui->hot_rect = rect;
                        ui->hot_is_on_top = ui->active_window == UI_WINDOW_top;
                        ui->input->mouse_position_used = 1;
                    }
                }
            }
        }
    }
    
    return result;
}

internal UIWidget *
UIGetNextWidget(UI *ui, UIID expected_id)
{
    UIWidget *widget = ui->widgets + ui->widget_count;
    
    if(!UIIDEqual(widget->id, expected_id))
    {
        for(u32 i = ui->widget_count+1; i < ui->last_frame_widget_count; ++i)
        {
            if(UIIDEqual(ui->widgets[i].id, expected_id))
            {
                MemoryMove(ui->widgets+ui->widget_count, ui->widgets + i,
                           (ui->last_frame_widget_count - i) * sizeof(UIWidget));
            }
        }
    }
    
    ++ui->widget_count;
    return widget;
}

internal b32
_UITextButton(UI *ui, UIID id, v4 rect, char *text, b32 editor)
{
    b32 result = _UIUpdateClickableWidget(ui, id, rect, text, editor);
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = editor ? UI_WIDGET_editor_button : UI_WIDGET_button;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
    return result;
}

internal b32
UIButton(UI *ui, char *text)
{
    v4 rect = {0};
    v4 color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, text, &rect, &color);
    UIID id = UIGenerateID(ui, text);
    return _UITextButton(ui, id, rect, text, 0);
}

internal b32
_UIImageButton(UI *ui, UIID id, v4 rect, char *text, b32 editor, Texture *texture, v4 source)
{
    b32 result = _UIUpdateClickableWidget(ui, id, rect, text, editor);
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = editor ? UI_WIDGET_editor_image_button : UI_WIDGET_button;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
    widget->image_button.texture = texture;
    widget->image_button.source = source;
    return result;
}

internal b32
_UIToggler(UI *ui, UIID id, v4 rect, char *text, b32 editor, b32 value)
{
    b32 clicked = _UIUpdateClickableWidget(ui, id, rect, text, editor);
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = editor ? UI_WIDGET_editor_toggler : UI_WIDGET_toggler;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
    if(clicked)
    {
        value = !value;
    }
    widget->toggler.toggled = value;
    return value;
}

internal b32
UIToggler(UI *ui, char *text, b32 value)
{
    v4 rect = {0};
    v4 color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, text, &rect, &color);
    UIID id = UIGenerateID(ui, text);
    return _UIToggler(ui, id, rect, text, 0, value);
}

internal f32
_UISlider(UI *ui, UIID id, v4 rect, char *text, f32 value, f32 low,
          f32 high, b32 editor)
{
    b32 result = 0;
    b32 keyboard_mode = !ui->mouse_mode;
    b32 cursor_over =
        V4RectHasPoint(rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)) &&
        V4RectHasPoint(ui->current_auto_layout_state.clip, v2(ui->input->mouse_position.x, ui->input->mouse_position.y));
    
    if(UIIDEqual(ui->active, id))
    {
        if(keyboard_mode)
        {
#if 0 // TODO(rjf)
            if(UIIDEqual(id, ui->hot))
            {
                if(ui->right_hold)
                {
                    value += (high - low) * ui->delta_t * 0.5f;
                }
                else if(ui->left_hold)
                {
                    value -= (high - low) * ui->delta_t * 0.5f;
                }
            }
#endif
        }
        else
        {
            if(ui->input->left_mouse_down)
            {
                value = (high - low) * ((ui->input->mouse_position.x - rect.x) / rect.width) + low;
            }
            else
            {
                ui->active = UIIDNull();
                if(!cursor_over)
                {
                    ui->hot = UIIDNull();
                }
            }
        }
    }
    else if(ui->allowed_window == ui->active_window || ui->active_window == UI_WINDOW_top)
    {
        if(!UIIDEqual(ui->active, id))
        {
            if(UIIDEqual(ui->hot, id))
            {
                if(ui->input->left_mouse_down)
                {
                    ui->active = id;
                    ui->input->mouse_position_used = 1;
                    ui->input->mouse_buttons_used = 1;
                }
                else if(!cursor_over)
                {
                    ui->hot = UIIDNull();
                }
            }
            else
            {
                if((UIIDIsNull(ui->hot) || !ui->hot_is_on_top) && cursor_over && !ui->input->left_mouse_down)
                {
                    ui->hot = id;
                    ui->hot_is_on_top = ui->active_window == UI_WINDOW_top;
                    ui->hot_rect = rect;
                    ui->input->mouse_position_used = 1;
                }
            }
        }
    }
    
    if(value < low)
    {
        value = low;
    }
    
    if(value > high)
    {
        value = high;
    }
    
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = editor ? UI_WIDGET_editor_slider : UI_WIDGET_slider;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
    widget->slider.slider_value = value;
    widget->slider.slider_percentage = (value - low) / (high - low);
    
    return value;
}

internal f32
UISlider(UI *ui, char *text, f32 value, f32 low, f32 high)
{
    v4 rect;
    v4 text_color;
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, text, &rect, &text_color);
    UIID id = UIGenerateID(ui, text);
    return _UISlider(ui, id, rect, text, value, low, high, 0);
}

internal char *
_UILineEdit(UI *ui, UIID id, v4 rect, char *text,
            char *edit_text, u32 edit_text_max, b32 editor)
{
    b32 result = 0;
    
    if(ui->allowed_window == ui->active_window || ui->active_window == UI_WINDOW_top)
    {
        
        b32 keyboard_mode = !ui->mouse_mode;
        
        if(keyboard_mode)
        {
            if(UIIDEqual(id, ui->hot))
            {
                ui->active = id;
            }
        }
        else
        {
            b32 cursor_over =
                V4RectHasPoint(rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)) &&
                V4RectHasPoint(ui->current_auto_layout_state.clip, v2(ui->input->mouse_position.x, ui->input->mouse_position.y));
            
            if(UIIDEqual(ui->active, id))
            {
                if(ui->input->left_mouse_down)
                {
                    if(cursor_over)
                    {
                        // TODO(rjf): Text selection?
                    }
                    else
                    {
                        ui->active = UIIDNull();
                    }
                }
            }
            else
            {
                if(UIIDEqual(ui->hot, id))
                {
                    if(ui->input->left_mouse_down && cursor_over)
                    {
                        ui->active = id;
                        ui->input->mouse_position_used = 1;
                        ui->input->mouse_buttons_used = 1;
                    }
                    else if(!cursor_over)
                    {
                        ui->hot = UIIDNull();
                    }
                }
                else
                {
                    if((UIIDIsNull(ui->hot) || !ui->hot_is_on_top) && cursor_over && !ui->input->left_mouse_down)
                    {
                        ui->hot = id;
                        ui->hot_is_on_top = ui->active_window == UI_WINDOW_top;
                        ui->hot_rect = rect;
                        ui->input->mouse_position_used = 1;
                    }
                }
            }
        }
    }
    
    if(UIIDEqual(ui->active, id))
    {
        TargetTextForInput(ui->input, edit_text, edit_text_max, CalculateCStringLength(edit_text));
    }
    
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = editor ? UI_WIDGET_editor_line_edit : UI_WIDGET_line_edit;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
    widget->line_edit.edit_text = MakeStringOnMemoryArena(ui->widget_arena, "%s", edit_text);
    
    return edit_text;
}

internal char *
UILineEdit(UI *ui, char *text, char *edit_text, u32 edit_text_max)
{
    v4 rect;
    v4 text_color;
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, text, &rect, &text_color);
    UIID id = UIGenerateID(ui, text);
    return _UILineEdit(ui, id, rect, text, edit_text, edit_text_max, 0);
}

internal void
UITitle(UI *ui, char *text)
{
    v4 rect;
    v4 color;
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, text, &rect, &color);
    UIID id = UIIDNonInteractable();
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = UI_WIDGET_title;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
}

internal void
UILabel(UI *ui, char *text)
{
    v4 rect;
    v4 color;
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, text, &rect, &color);
    UIID id = UIIDNonInteractable();
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = UI_WIDGET_label;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
}

internal void
UIDivider(UI *ui)
{
    v4 rect;
    v4 color;
    
    if(ui->current_auto_layout_state.is_column)
    {
        UIPushHeight(ui, ui->current_auto_layout_state.size.y/2);
    }
    else
    {
        UIPushHeight(ui, ui->current_auto_layout_state.size.x/16);
    }
    
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, "", &rect, &color);
    UIID id = UIIDNonInteractable();
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = UI_WIDGET_divider;
    UIWidgetInit(ui, widget, widget_type, id, rect, 0, ui->active_window);
    
    if(ui->current_auto_layout_state.is_column)
    {
        UIPopHeight(ui);
    }
    else
    {
        UIPopWidth(ui);
    }
    
}

internal b32
UIEditorCloseButton(UI *ui)
{
    v4 rect = {0};
    v4 color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, "x", &rect, &color);
    UIID id = UIGenerateID(ui, "Close");
    b32 clicked = _UITextButton(ui, id, rect, "x", 1);
    ui->widgets[ui->widget_count-1].type = UI_WIDGET_editor_close_button;
    return clicked;
}

internal void
UIWindowBegin(UI *ui, char *title, v4 rect, i32 flags, ...)
{
    b32 *open_value_ptr = 0;
    if(flags & UI_WINDOW_FLAG_closable)
    {
        va_list args;
        va_start(args, flags);
        open_value_ptr = va_arg(args, b32 *);
        va_end(args);
    }
    
    HardAssert(ui->window_count < UI_WINDOW_MAX);
    UIWindow *window = 0;
    b32 first_frame_of_windows_existence = 0;
    
    u32 title_hash = HashCString(title);
    u32 hash_value = title_hash % UI_WINDOW_MAX;
    u32 original_hash_value = hash_value;
    
    UIWindow *open_slot = 0;
    
    for(;;)
    {
        if(ui->windows[hash_value].active && !ui->windows[hash_value].deleted)
        {
            if(CStringMatchCaseSensitive(ui->windows[hash_value].title, title))
            {
                open_slot = ui->windows + hash_value;
                first_frame_of_windows_existence = 0;
                break;
            }
        }
        else
        {
            open_slot = ui->windows + hash_value;
            first_frame_of_windows_existence = 1;
            if(!ui->windows[hash_value].deleted)
            {
                break;
            }
        }
        
        ++hash_value;
        if(hash_value >= UI_WINDOW_MAX)
        {
            hash_value = 0;
        }
        if(hash_value == original_hash_value)
        {
            break;
        }
    }
    
    window = open_slot;
    
    if(window)
    {
        CopyCStringToFixedSizeBuffer(window->title, sizeof(window->title), title);
        window->title_hash = title_hash;
        if(first_frame_of_windows_existence)
        {
            ++ui->window_count;
            window->rect = rect;
            window->view_offset = v2(0, 0);
            window->target_view_offset = v2(0, 0);
            
            MemoryMove(ui->window_order+1, ui->window_order, sizeof(ui->window_order[0])*ui->window_count);
            ui->window_order[0] = hash_value;
        }
        else
        {
            window->rect.width = rect.width;
            window->rect.height = rect.height;
        }
        
        window->flags = flags;
        window->widget_index_start = ui->widget_count;
        window->active = 1;
        window->stale = 0;
        window->deleted = 0;
        window->open_value_ptr = open_value_ptr;
        
        window->target_view_offset.x = ClampF32(window->target_view_offset.x, 0, window->target_view_max.x);
        window->target_view_offset.y = ClampF32(window->target_view_offset.y, 0, window->target_view_max.y);
        
        window->target_view_max.x = 0;
        window->target_view_max.y = 0;
        
        window->view_offset.x += (window->target_view_offset.x - window->view_offset.x) * ui->delta_t * 16.f;
        window->view_offset.y += (window->target_view_offset.y - window->view_offset.y) * ui->delta_t * 16.f;
        
        if(window == ui->allowed_window)
        {
            ui->input->mouse_position_used = 1;
            window->target_view_offset.y -= 1.f * ui->input->mouse_scroll.y;
            
            if(window->flags & UI_WINDOW_FLAG_movable && UIIDIsNull(ui->hot))
            {
                if(ui->dragging_window == window)
                {
                    if(ui->input->left_mouse_down)
                    {
                        ui->input->mouse_position_used = 1;
                        ui->input->mouse_buttons_used = 1;
                        
                        window->rect.x += ui->input->mouse_position.x - ui->last_mouse.x;
                        window->rect.y += ui->input->mouse_position.y - ui->last_mouse.y;
                        
                        for(u32 i = 0; i < ui->window_count; ++i)
                        {
                            if(ui->windows + ui->window_order[i] == window)
                            {
                                u32 window_index = ui->window_order[i];
                                MemoryMove(ui->window_order+1, ui->window_order, sizeof(ui->window_order[0])*i);
                                ui->window_order[0] = window_index;
                                break;
                            }
                        }
                    }
                    else
                    {
                        ui->dragging_window = 0;
                    }
                }
                else if(!ui->dragging_window)
                {
                    if(V4RectHasPoint(v4(window->rect.x, window->rect.y,
                                         window->rect.width, 30), v2(ui->input->mouse_position.x, ui->input->mouse_position.y)))
                    {
                        if(UIIDEqual(ui->hot, UIIDNull()) && ui->input->left_mouse_pressed)
                        {
                            ui->dragging_window = window;
                            ui->input->mouse_position_used = 1;
                            ui->input->mouse_buttons_used = 1;
                        }
                    }
                }
            }
        }
    }
    else
    {
        HardAssert("No window slots available" == 0);
    }
    
    ui->active_window = window;
    
    if(window->flags & UI_WINDOW_FLAG_closable)
    {
        UIPushPosition(ui, v2(window->rect.x + window->rect.width - 30,
                              window->rect.y));
        UIPushSize(ui, v2(30, 30));
        if(UIEditorCloseButton(ui))
        {
            *window->open_value_ptr = 0;
        }
        UIPopSize(ui);
        UIPopPosition(ui);
    }
    
    if(flags & UI_WINDOW_FLAG_title_bar)
    {
        UIPushPosition(ui, v2(window->rect.x - window->view_offset.x, window->rect.y + 30 - window->view_offset.y));
        UIPushClip(ui, v4(window->rect.x, window->rect.y + 31, window->rect.width, window->rect.height - 31));
    }
    else
    {
        UIPushPosition(ui, v2(window->rect.x - window->view_offset.x, window->rect.y - window->view_offset.y));
        UIPushClip(ui, window->rect);
    }
}

internal void
UIWindowEnd(UI *ui)
{
    SoftAssert(ui->active_window != 0);
    ui->active_window->widget_index_end = ui->widget_count;
    
    if(UIIDEqual(ui->hot, UIIDNull()))
    {
        if(V4RectHasPoint(ui->active_window->rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)))
        {
            ui->input->mouse_position_used = 1;
            ui->input->mouse_buttons_used = 1;
        }
    }
    
    ui->active_window = 0;
    
    UIPopPosition(ui);
    UIPopClip(ui);
}

internal void
UICanvas(UI *ui, char *text,
         UICanvasUpdateCallback *Update, void *update_user_data,
         UICanvasRenderCallback *Render, void *render_user_data)
{
    v4 rect;
    v4 color;
    UIGetAndUpdateAutoLayoutState(ui, ui->main_font, 0, &rect, &color);
    UIID id = UIGenerateID(ui, text);
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = UI_WIDGET_canvas;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
    widget->canvas.Update = Update;
    widget->canvas.update_user_data = update_user_data;
    widget->canvas.Render = Render;
    widget->canvas.render_user_data = render_user_data;
    
    widget->canvas.Update(text, rect, v2(ui->input->mouse_position.x - rect.x, ui->input->mouse_position.y - rect.y), update_user_data);
}

internal b32
UIEditorButton(UI *ui, char *text)
{
    v4 rect = {0};
    v4 color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &color);
    UIID id = UIGenerateID(ui, text);
    return _UITextButton(ui, id, rect, text, 1);
}

internal b32
UIEditorImageButton(UI *ui, char *text, Texture *texture, v4 source)
{
    v4 rect = {0};
    v4 color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &color);
    UIID id = UIGenerateID(ui, text);
    return _UIImageButton(ui, id, rect, text, 1, texture, source);
}

internal b32
UIEditorToggler(UI *ui, char *text, b32 value)
{
    v4 rect = {0};
    v4 color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &color);
    UIID id = UIGenerateID(ui, text);
    return _UIToggler(ui, id, rect, text, 1, value);
}

internal f32
UIEditorSlider(UI *ui, char *text, f32 value, f32 low, f32 high)
{
    v4 rect;
    v4 text_color;
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &text_color);
    UIID id = UIGenerateID(ui, text);
    return _UISlider(ui, id, rect, text, value, low, high, 1);
}

internal char *
UIEditorLineEdit(UI *ui, char *text, char *edit_text, u32 edit_text_max)
{
    v4 rect;
    v4 text_color;
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &text_color);
    UIID id = UIGenerateID(ui, text);
    return _UILineEdit(ui, id, rect, text, edit_text, edit_text_max, 1);
}

internal b32
UIEditorDropdown(UI *ui, char *text)
{
    v4 rect = {0};
    v4 color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &color);
    UIID id = UIGenerateID(ui, text);
    b32 clicked = _UITextButton(ui, id, rect, text, 1);
    b32 dropdown_open = 0;
    i32 open_dropdown_index = -1;
    
    UIWidget *widget = ui->widgets + ui->widget_count - 1;
    
    for(u32 i = 0; i < ui->open_dropdown_stack_size; ++i)
    {
        if(UIIDEqual(ui->open_dropdown_stack[i].widget_id, id))
        {
            dropdown_open = 1;
            open_dropdown_index = (i32)i;
            break;
        }
    }
    
    if(clicked)
    {
        if(dropdown_open)
        {
            dropdown_open = 0;
            ui->open_dropdown_stack_size = (u32)open_dropdown_index;
        }
        else
        {
            dropdown_open = 1;
            
            if(ui->active_dropdown)
            {
                
                for(u32 i = 0; i < ui->open_dropdown_stack_size; ++i)
                {
                    if(UIIDEqual(ui->open_dropdown_stack[i].widget_id, ui->active_dropdown->id))
                    {
                        ui->open_dropdown_stack_size = i+1;
                    }
                }
                
            }
            else
            {
                ui->open_dropdown_stack_size = 0;
            }
            
            open_dropdown_index = (i32)ui->open_dropdown_stack_size;
            ui->open_dropdown_stack[ui->open_dropdown_stack_size].widget_id = id;
            ui->open_dropdown_stack[ui->open_dropdown_stack_size++].open_transition = 0.f;
        }
    }
    
    ui->active_dropdown_stack[ui->active_dropdown_stack_size++] = ui->active_dropdown;
    ui->active_dropdown = widget;
    
    if(ui->current_auto_layout_state.is_column)
    {
        UIPushPosition(ui, v2(rect.width, -rect.height));
    }
    else
    {
        UIPushPosition(ui, v2(-rect.width, rect.height));
    }
    UIPushGroupMode(ui, 1);
    
    if(dropdown_open && ui->open_dropdown_stack_size)
    {
        ui->open_dropdown_stack[open_dropdown_index].open_transition +=
            (1 - ui->open_dropdown_stack[open_dropdown_index].open_transition) * ui->delta_t * 20.f;
    }
    
    f32 transition = 0.f;
    if(ui->open_dropdown_stack_size)
    {
        transition = ui->open_dropdown_stack[open_dropdown_index].open_transition;
    }
    UIPushSize(ui, v2(rect.width, rect.height*transition));
    
    return dropdown_open;
}

internal void
UICloseCurrentDropdownTree(UI *ui)
{
    ui->open_dropdown_stack_size = 0;
}

internal void
UIDropdownEnd(UI *ui)
{
    UIPopGroupMode(ui);
    UIPopPosition(ui);
    UIPopSize(ui);
    
    if(ui->active_dropdown_stack_size > 0)
    {
        ui->active_dropdown = ui->active_dropdown_stack[--ui->active_dropdown_stack_size];
    }
    else
    {
        ui->active_dropdown = 0;
    }
}

internal v3
_UIEditorColorPicker(UI *ui, UIID id, v4 rect, char *text, v3 color)
{
    v3 color_hsv = RGBToHSV(color);
    
    b32 keyboard_mode = !ui->mouse_mode;
    b32 changing = 0;
    
    if(ui->active_window == ui->allowed_window || ui->active_window == UI_WINDOW_top)
    {
        if(keyboard_mode)
        {
            if(UIIDEqual(id, ui->hot))
            {
                changing = 1;
            }
        }
        else
        {
            b32 cursor_over =
                V4RectHasPoint(rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)) &&
                V4RectHasPoint(ui->current_auto_layout_state.clip, v2(ui->input->mouse_position.x, ui->input->mouse_position.y));
            
            if(UIIDEqual(ui->active, id))
            {
                if(ui->input->left_mouse_down)
                {
                    changing = 1;
                }
                else
                {
                    ui->active = UIIDNull();
                    if(!cursor_over)
                    {
                        ui->hot = UIIDNull();
                    }
                }
            }
            else
            {
                if(UIIDEqual(ui->hot, id))
                {
                    if(ui->input->left_mouse_down)
                    {
                        ui->active = id;
                    }
                    else if(!cursor_over)
                    {
                        ui->hot = UIIDNull();
                    }
                }
                else
                {
                    if((UIIDIsNull(ui->hot) || !ui->hot_is_on_top) && cursor_over && !ui->input->left_mouse_down)
                    {
                        ui->hot = id;
                        ui->hot_is_on_top = ui->active_window == UI_WINDOW_top;
                        ui->hot_rect = rect;
                    }
                }
            }
        }
    }
    
    v4 h_selector_rect = UIGetColorPickerHueSelectorRect(rect);
    v4 sv_selector_rect = UIGetColorPickerSaturationValueSelectorRect(rect);
    
    UIWidgetType widget_type = UI_WIDGET_editor_color_picker;
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    
    if(UIIDEqual(widget->id, id))
    {
        if(changing)
        {
            if(!widget->color_picker.was_selecting)
            {
                if(keyboard_mode)
                {
                    // TODO(rjf): Keyboard controls
                }
                else
                {
                    if(V4RectHasPoint(h_selector_rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)))
                    {
                        widget->color_picker.selecting_hue = 1;
                    }
                    else if(V4RectHasPoint(sv_selector_rect, v2(ui->input->mouse_position.x, ui->input->mouse_position.y)))
                    {
                        widget->color_picker.selecting_hue = 0;
                    }
                }
            }
            
            widget->color_picker.was_selecting = 1;
            
            if(widget->color_picker.was_selecting)
            {
                if(keyboard_mode)
                {
                    // TODO(rjf): Keyboard controls
                }
                else
                {
                    if(widget->color_picker.selecting_hue)
                    {
                        color_hsv.x = (ui->input->mouse_position.y - h_selector_rect.y) / h_selector_rect.height;
                    }
                    else
                    {
                        color_hsv.y = (ui->input->mouse_position.x - sv_selector_rect.x) / sv_selector_rect.width;
                        color_hsv.z = 1 - (ui->input->mouse_position.y - sv_selector_rect.y) / sv_selector_rect.height;
                    }
                }
            }
            
            color_hsv.x = ClampF32(color_hsv.x, 0.001f, 0.999f);
            color_hsv.y = ClampF32(color_hsv.y, 0.001f, 0.999f);
            color_hsv.z = ClampF32(color_hsv.z, 0.001f, 0.999f);
        }
        else
        {
            widget->color_picker.selecting_hue = 0;
            widget->color_picker.was_selecting = 0;
        }
        
        color = HSVToRGB(color_hsv);
        
        widget->color_picker.rgb = color;
        widget->color_picker.hsv = color_hsv;
    }
    else
    {
        widget->color_picker.rgb = color;
        widget->color_picker.hsv = color_hsv;
        widget->color_picker.selecting_hue = 0;
        widget->color_picker.was_selecting = 0;
    }
    
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
    
    return color;
}

internal v3
UIEditorColorPicker(UI *ui, char *text, v3 color)
{
    UIDivider(ui);
    UIPushSize(ui, v2(ui->current_auto_layout_state.size.x, ui->current_auto_layout_state.size.x));
    v4 rect = {0};
    v4 text_color = {0};
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &text_color);
    UIID id = UIGenerateID(ui, text);
    UIPopSize(ui);
    UIDivider(ui);
    return _UIEditorColorPicker(ui, id, rect, text, color);
}

internal void
UIEditorTitle(UI *ui, char *text)
{
    v4 rect;
    v4 color;
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &color);
    UIID id = UIIDNonInteractable();
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = UI_WIDGET_editor_title;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
}

internal void
UIEditorLabel(UI *ui, char *text)
{
    v4 rect;
    v4 color;
    UIGetAndUpdateAutoLayoutState(ui, ui->editor_font, text, &rect, &color);
    UIID id = UIIDNonInteractable();
    HardAssert(ui->widget_count < UI_WIDGET_MAX);
    UIWidget *widget = UIGetNextWidget(ui, id);
    UIWidgetType widget_type = UI_WIDGET_editor_label;
    UIWidgetInit(ui, widget, widget_type, id, rect, text, ui->active_window);
}

internal void
UIBeginAlwaysOnTop(UI *ui)
{
    ui->active_window = UI_WINDOW_top;
}

internal void
UIEndAlwaysOnTop(UI *ui)
{
    ui->active_window = 0;
}