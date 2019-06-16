typedef enum SettingsMenuState SettingsMenuState;
enum SettingsMenuState
{
    SETTINGS_MENU_main,
    SETTINGS_MENU_controls,
    SETTINGS_MENU_audio,
    SETTINGS_MENU_graphics,
    SETTINGS_MENU_display,
};

typedef struct SettingsMenu SettingsMenu;
struct SettingsMenu
{
    SettingsMenuState state;
    i32 selected_control;
};

internal b32
SettingsMenuUpdate(SettingsMenu *settings_menu);