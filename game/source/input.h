typedef struct Input Input;
struct Input
{
    v2 mouse_position;
    v2 mouse_scroll;
    b32 mouse_position_used;
    b32 mouse_buttons_used;
    b32 left_mouse_pressed;
    b32 left_mouse_down;
    b32 right_mouse_pressed;
    b32 right_mouse_down;
    char **target_text;
    u32 *target_text_max;
    u32 *target_text_edit_position;
};

internal void
InputBeginFrame(Input *input, Platform *platform)
{
    input->mouse_position.x = platform->mouse_x;
    input->mouse_position.y = platform->mouse_y;
    input->mouse_scroll.x = platform->mouse_scroll_x;
    input->mouse_scroll.y = platform->mouse_scroll_y;
    input->mouse_position_used = 0;
    input->mouse_buttons_used = 0;
    input->left_mouse_pressed = platform->left_mouse_pressed;
    input->left_mouse_down = platform->left_mouse_down;
    input->right_mouse_pressed = platform->right_mouse_pressed;
    input->right_mouse_down = platform->right_mouse_down;
    input->target_text = &platform->target_text;
    input->target_text_max = &platform->target_text_max_characters;
    input->target_text_edit_position = &platform->target_text_edit_pos;
}

internal void
InputEndFrame(Input *input)
{}

internal void
TargetTextForInput(Input *input, char *target, u32 max, u32 edit_position)
{
    *input->target_text = target;
    *input->target_text_max = max;
    *input->target_text_edit_position = edit_position;
}

internal b32
CaptureLeftMousePressed(Input *input)
{
    b32 result = input->left_mouse_pressed;
    input->left_mouse_pressed = 0;
    return result;
}

internal b32
CaptureLeftMouseDown(Input *input)
{
    b32 result = input->left_mouse_down;
    input->left_mouse_down = 0;
    return result;
}

internal b32
CaptureRightMousePressed(Input *input)
{
    b32 result = input->right_mouse_pressed;
    input->right_mouse_pressed = 0;
    return result;
}

internal b32
CaptureRightMouseDown(Input *input)
{
    b32 result = input->right_mouse_down;
    input->right_mouse_down = 0;
    return result;
}