#define UI_WIDGET_MAX 1024
#define UI_WINDOW_MAX 128
#define UI_WIDGET_DEFAULT_WIDTH 256
#define UI_WIDGET_DEFAULT_HEIGHT 48
#define UI_TEXT_ARENA_SIZE Kilobytes(64)
#define UI_STACK_MAX 16
#define UI_WINDOW_FLAG_movable      0x01
#define UI_WINDOW_FLAG_closable     0x02
#define UI_WINDOW_FLAG_title_bar    0x04
#define UI_WINDOW_top ((UIWindow *)(0x2272272272272279))

typedef struct UIID
{
    i32 primary;
    i32 secondary;
}
UIID;

typedef enum UIWidgetType
{
    UI_WIDGET_button,
    UI_WIDGET_toggler,
    UI_WIDGET_slider,
    UI_WIDGET_line_edit,
    UI_WIDGET_label,
    UI_WIDGET_title,
    UI_WIDGET_divider,
    UI_WIDGET_canvas,
    
    UI_WIDGET_editor_button,
    UI_WIDGET_editor_close_button,
    UI_WIDGET_editor_image_button,
    UI_WIDGET_editor_toggler,
    UI_WIDGET_editor_slider,
    UI_WIDGET_editor_line_edit,
    UI_WIDGET_editor_color_picker,
    UI_WIDGET_editor_note_picker,
    UI_WIDGET_editor_tile_select,
    UI_WIDGET_editor_label,
    UI_WIDGET_editor_title,
    UI_WIDGET_editor_divider,
    UI_WIDGET_editor_rect,
}
UIWidgetType;

typedef struct UIWindow
{
    char title[64];
    u32 title_hash;
    v4 rect;
    i32 flags;
    u32 widget_index_start;
    u32 widget_index_end;
    b32 active;
    b32 stale;
    b32 deleted;
    b32 *open_value_ptr;
    v2 view_offset;
    v2 target_view_offset;
    v2 target_view_max;
}
UIWindow;

typedef void UICanvasUpdateCallback(char *name, v4 rect, v2 mouse, void *user_data);
typedef void UICanvasRenderCallback(char *name, v4 rect, v2 mouse, void *user_data);

typedef struct UIWidget
{
    UIWidgetType type;
    UIID id;
    v4 rect;
    v4 clip;
    f32 hot_transition;
    f32 active_transition;
    String text;
    v4 text_color;
    f32 text_scale;
    UIWindow *parent_window;
    
    union
    {
        
        struct ImageButton
        {
            Texture *texture;
            v4 source;
        }
        image_button;
        
        struct Toggler
        {
            b32 toggled;
            f32 toggle_transition;
        }
        toggler;
        
        struct Slider
        {
            f32 slider_value;
            f32 slider_percentage;
        }
        slider;
        
        struct LineEdit
        {
            String edit_text;
            f32 view_offset;
        }
        line_edit;
        
        struct TileSelect
        {
            b32 was_selecting;
            v4 tilemap_source;
            i16 selection_x0;
            i16 selection_x1;
            i16 selection_y0;
            i16 selection_y1;
            Texture *tilemap_texture;
        }
        tile_select;
        
        struct ColorPicker
        {
            v3 rgb;
            v3 hsv;
            b32 selecting_hue;
            b32 was_selecting;
        }
        color_picker;
        
        struct NotePicker
        {
            i32 selected_note;
        }
        note_picker;
        
        struct Canvas
        {
            UICanvasUpdateCallback *Update;
            void *update_user_data;
            UICanvasRenderCallback *Render;
            void *render_user_data;
        }
        canvas;
        
    };
}
UIWidget;

typedef struct UIFrameData
{
    f32 delta_t;
    MemoryArena *widget_arena;
    Font *main_font;
    Font *editor_font;
    Input *input;
}
UIFrameData;

typedef struct UI
{
    
    // NOTE(rjf): Frame data
    UIID hot;
    v4 hot_rect;
    b32 hot_is_on_top;
    UIID active;
    Input *input;
    v2 last_mouse;
    b32 mouse_mode;
    f32 delta_t;
    MemoryArena *widget_arena;
    f32 caret_x_offset;
    Font *main_font;
    Font *editor_font;
    f32 render_w;
    f32 render_h;
    
    // NOTE(rjf): Widget data
    u32 widget_count;
    u32 last_frame_widget_count;
    UIWidget widgets[UI_WIDGET_MAX];
    u32 widget_id_counters[UI_WIDGET_MAX];
    
    // NOTE(rjf): Auto-layout and coloring information
    struct
    {
        v2 position;
        v2 size;
        b32 calculate_width_with_text;
        b32 calculate_height_with_text;
        v4 text_color;
        f32 text_scale;
        v4 clip;
        b32 is_column;
    }
    current_auto_layout_state;
    
    // NOTE(rjf): Stacks
    struct
    {
        u32 x_position_stack_size;
        struct
        {
            f32 x;
        }
        x_position_stack[UI_STACK_MAX];
        
        u32 y_position_stack_size;
        struct
        {
            f32 y;
        }
        y_position_stack[UI_STACK_MAX];
        
        u32 width_stack_size;
        struct
        {
            f32 width;
            b32 calculate_width_with_text;
        }
        width_stack[UI_STACK_MAX];
        
        u32 height_stack_size;
        struct
        {
            f32 height;
            b32 calculate_height_with_text;
        }
        height_stack[UI_STACK_MAX];
        
        u32 text_color_stack_size;
        struct
        {
            v4 color;
        }
        text_color_stack[UI_STACK_MAX];
        
        u32 text_scale_stack_size;
        struct
        {
            f32 scale;
        }
        text_scale_stack[UI_STACK_MAX];
        
        u32 group_mode_stack_size;
        struct
        {
            b32 is_column;
        }
        group_mode_stack[UI_STACK_MAX];
        
        u32 open_dropdown_stack_size;
        struct
        {
            UIID widget_id;
            f32 open_transition;
        }
        open_dropdown_stack[UI_STACK_MAX];
        
        u32 active_dropdown_stack_size;
        UIWidget *active_dropdown_stack[UI_STACK_MAX];
        
        u32 clip_stack_size;
        v4 clip_stack[UI_STACK_MAX];
    };
    
    // NOTE(rjf): Window data
    u32 window_count;
    UIWindow windows[UI_WINDOW_MAX];
    UIWindow *active_window;
    UIWindow *allowed_window;
    u32 window_order[UI_WINDOW_MAX];
    UIWindow *dragging_window;
    
    // NOTE(rjf): Dropdown data
    UIWidget *active_dropdown;
}
UI;
