typedef struct GameOverState GameOverState;
struct GameOverState
{
    f32 background_transition;
};

internal void
GameOverStateInit(GameOverState *state)
{
    state->background_transition = 1.f;
}

internal void
GameOverStateCleanUp(GameOverState *state)
{}

internal StateType
GameOverStateUpdate(GameOverState *state)
{
    StateType next_state = STATE_invalid;
    
    UIPushCenteredColumn(&app->ui, v2(350, 50), 4);
    {
        UITitle(&app->ui, "GAME OVER");
        UIDivider(&app->ui);
        
        if(UIButton(&app->ui, "Return to Main Menu"))
        {
            next_state = STATE_title;
        }
        
        if(UIButton(&app->ui, "Quit"))
        {
            platform->quit = 1;
        }
    }
    UIPopColumn(&app->ui);
    
    state->background_transition -= state->background_transition * app->delta_t;
    
    RendererPushFilledRect2D(&app->renderer, v4(state->background_transition, 0, 0, 1),
                             v4(0, 0, app->render_w, app->render_h));
    
    return next_state;
}