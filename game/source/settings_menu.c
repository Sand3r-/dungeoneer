
internal b32
SettingsMenuUpdate(SettingsMenu *settings_menu)
{
    b32 keep_open = 1;
    
    switch(settings_menu->state)
    {
        
        case SETTINGS_MENU_main:
        {
            
            UIPushColumn(&app->ui, v2(app->render_w/2 - 100, app->render_h/2 - 6*25), v2(200, 50));
            {
                UITitle(&app->ui, "SETTINGS");
                
                if(UIButton(&app->ui, "Controls"))
                {
                    settings_menu->state = SETTINGS_MENU_controls;
                }
                
                if(UIButton(&app->ui, "Audio"))
                {
                    settings_menu->state = SETTINGS_MENU_audio;
                }
                
                if(UIButton(&app->ui, "Graphics"))
                {
                    settings_menu->state = SETTINGS_MENU_graphics;
                }
                
                if(UIButton(&app->ui, "Display"))
                {
                    settings_menu->state = SETTINGS_MENU_display;
                }
                
                UIDivider(&app->ui);
                
                if(UIButton(&app->ui, "Back"))
                {
                    keep_open = 0;
                }
                
            }
            UIPopColumn(&app->ui);
            
            break;
        }
        
        case SETTINGS_MENU_controls:
        {
            
            UIPushColumn(&app->ui, v2(app->render_w/2 - 100, app->render_h/2 - 6*25), v2(200, 50));
            {
                UITitle(&app->ui, "CONTROLS");
                
                UIDivider(&app->ui);
                
                if(UIButton(&app->ui, "Back"))
                {
                    settings_menu->state = SETTINGS_MENU_main;
                }
                
            }
            UIPopColumn(&app->ui);
            
            break;
        }
        
        case SETTINGS_MENU_audio:
        {
            
            UIPushColumn(&app->ui, v2(app->render_w/2 - 100, app->render_h/2 - 6*25), v2(200, 50));
            {
                UITitle(&app->ui, "AUDIO");
                
                for(u32 i = 0; i < AUDIO_MAX; ++i)
                {
                    app->audio.volumes[i] = UISlider(&app->ui, AudioTypeName(i), app->audio.volumes[i], 0.f, 1.f);
                }
                
                UIDivider(&app->ui);
                
                if(UIButton(&app->ui, "Back"))
                {
                    settings_menu->state = SETTINGS_MENU_main;
                }
                
            }
            UIPopColumn(&app->ui);
            
            break;
        }
        
        case SETTINGS_MENU_graphics:
        {
            
            UIPushColumn(&app->ui, v2(app->render_w/2 - 100, app->render_h/2 - 6*25), v2(200, 50));
            {
                UITitle(&app->ui, "GRAPHICS");
                
                app->anti_aliasing = UIToggler(&app->ui, "Anti-Aliasing", app->anti_aliasing);
                app->shadows = UIToggler(&app->ui, "Shadows", app->shadows);
                
                UIDivider(&app->ui);
                
                if(UIButton(&app->ui, "Back"))
                {
                    settings_menu->state = SETTINGS_MENU_main;
                }
                
            }
            UIPopColumn(&app->ui);
            
            break;
        }
        
        case SETTINGS_MENU_display:
        {
            
            UIPushColumn(&app->ui, v2(app->render_w/2 - 100, app->render_h/2 - 6*25), v2(200, 50));
            {
                UITitle(&app->ui, "DISPLAY");
                
                platform->fullscreen = UIToggler(&app->ui, "Fullscreen", platform->fullscreen);
                platform->vsync = UIToggler(&app->ui, "Vertical-Sync", platform->vsync);
                platform->target_frames_per_second = UISlider(&app->ui, "FPS", platform->target_frames_per_second, 30.f, 240.f);
                
                UIDivider(&app->ui);
                
                if(UIButton(&app->ui, "Back"))
                {
                    settings_menu->state = SETTINGS_MENU_main;
                }
                
            }
            UIPopColumn(&app->ui);
            
            break;
        }
        
    }
    
    return keep_open;
}