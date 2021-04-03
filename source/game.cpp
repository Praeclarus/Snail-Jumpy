

global f32 CompletionCooldown;

internal void
GameProcessKeyDown(os_event *Event){
    switch((u32)Event->Key){
        case KeyCode_Escape: ChangeState(GameMode_MainGame, "Overworld"); break;
        case 'E':            ToggleWorldEditor(); break;
        case 'P': {
            CurrentWorld->Flags |= WorldFlag_IsCompleted;
            ChangeState(GameMode_MainGame, "Overworld");
        }break;
#ifdef SNAIL_JUMPY_DEBUG_BUILD
        case '=': GameCamera.MoveFactor += 0.05f; break;
        case '-': GameCamera.MoveFactor -= 0.05f; break;
#endif
    }
}

internal void
GameProcessInput(){
    os_event Event;
    while(PollEvents(&Event)){
        EntityManager.ProcessEvent(&Event);
        switch(Event.Kind){
            case OSEventKind_KeyDown: GameProcessKeyDown(&Event); break;
        }
        ProcessDefaultEvent(&Event);
    }
}

internal void
UpdateAndRenderMainGame(){
    GameProcessInput();
    
    Renderer.NewFrame(&TransientStorageArena, V2S(OSInput.WindowSize));
    Renderer.ClearScreen(Color(0.4f, 0.5f, 0.45f, 1.0f));
    GameCamera.Update();
    
    EntityManager.UpdateAndRenderEntities(&GameCamera);
    
    player_entity *Player = EntityManager.Player;
    // Gate
    {
        v2 P = v2{15.25f, 3.25f};
        v2 DrawP = P;
        v2 Radius = 0.5f*TILE_SIZE;
        RenderRect(CenterRect(DrawP, TILE_SIZE), 0.0f, ORANGE, &GameCamera);
        v2 PlayerMin = Player->Physics->P-(RectSize(EntityManager.Player->Bounds)/2);
        v2 PlayerMax = Player->Physics->P+(RectSize(EntityManager.Player->Bounds)/2);
        if((P.X-Radius.X <= PlayerMax.X)  &&
           (PlayerMin.X  <= P.X+Radius.X) &&
           (P.Y-Radius.Y <= PlayerMax.Y)  &&
           (PlayerMin.Y  <= P.Y+Radius.Y)){
            u32 RequiredCoins = CurrentWorld->CoinsRequired;
            if((u32)Score >= RequiredCoins){
                if(CompletionCooldown == 0.0f){
                    CompletionCooldown = 3.0f;
                }
            }else{
                v2 TopCenter = v2{
                    OSInput.WindowSize.Width/2, OSInput.WindowSize.Height/2
                };
                RenderCenteredString(&MainFont, GREEN, TopCenter, -0.9f,
                                     "You need: %u more coins!", 
                                     RequiredCoins-Score);
            }
        }
    }
    
    if(CompletionCooldown > 0.0f){
        f32 Advance =
            GetFormatStringAdvance(&MainFont, 
                                   "Level completed");
        v2 TopCenter = v2{
            OSInput.WindowSize.Width/2, OSInput.WindowSize.Height/2
        };
        color Color = GREEN;
        if(CompletionCooldown > 0.5f*3.0f){
            Color.A = 2.0f*(1.0f - CompletionCooldown/3.0f);
        }else if(CompletionCooldown < 0.3f*3.0f){
            Color.A = 2.0f * CompletionCooldown/3.0f;
        }
        RenderFormatString(&MainFont, Color, 
                           TopCenter.X-(0.5f*Advance), TopCenter.Y, -0.9f,
                           "Level completed!");
        
        CompletionCooldown -= OSInput.dTime;
        if(CompletionCooldown < 0.00001f){
            CurrentWorld->Flags |= WorldFlag_IsCompleted;
            CompletionCooldown = 0.0f;
            ChangeState(GameMode_MainGame, "Overworld");
        }
    }
    
    camera DummyCamera = {}; DummyCamera.MetersToPixels = GameCamera.MetersToPixels;
    // Weapon charge bar
    {
        v2 Min = v2{0.1f, 0.1f};
        v2 Max = Min;
        f32 Percent = 0.0f;
        Percent = EntityManager.Player->WeaponChargeTime;
        Max.X += 4.0f*Percent;
        Max.Y += 0.2f;
        RenderRect(Rect(Min, Max), -1.0f, Color(1.0f, 0.0f, 1.0f, 0.9f),
                   &DummyCamera);
    }
    
    // Health display
    {
        v2 P = v2{0.2f, 0.8f};
        f32 XAdvance = 0.3f;
        
        u32 FullHearts = Player->Health / 3;
        u32 Remainder = Player->Health % 3;
        
        Assert(FullHearts <= 3);
        u32 I;
        for(I = 0; I < FullHearts; I++){
            RenderFrameOfSpriteSheet( &DummyCamera, "heart", 0, P, -0.9f);
            P.X += XAdvance;
        }
        
        if(Remainder > 0){
            Remainder = 3 - Remainder;
            RenderFrameOfSpriteSheet( &DummyCamera, "heart", Remainder, P, -0.9f);
            P.X += XAdvance;
            I++;
        }
        
        if(I < 3){
            for(u32 J = 0; J < 3-I; J++){
                RenderFrameOfSpriteSheet(&DummyCamera, "heart", 3, P, -0.9f);
                P.X += XAdvance;
            }
        }
        
        RenderFormatString(&DebugFont, BLACK, 
                           GameCamera.MetersToPixels*P.X, GameCamera.MetersToPixels*P.Y,
                           -2.0f, "Health: %d", Player->Health);
    }
    
    //~ Debug UI
    RenderFormatString(&MainFont, GREEN, 100, OSInput.WindowSize.Height-100,
                       -0.9f, "Score: %u", Score);
    
    DEBUGRenderOverlay();
    Renderer.RenderToScreen();
}