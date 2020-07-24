

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
    }
}

internal void
UpdateAndRenderMainGame(){
    //TIMED_FUNCTION();
    
    
    GameProcessInput();
    
    render_group RenderGroup;
    
    InitializeRenderGroup(&TransientStorageArena, &RenderGroup, Kilobytes(16));
    
    RenderGroup.BackgroundColor = {0.5f, 0.5f, 0.5f, 1.0f};
    RenderGroup.OutputSize = OSInput.WindowSize;
    //RenderGroup.MetersToPixels = 60.0f / 0.5f;
    RenderGroup.MetersToPixels = Minimum((OSInput.WindowSize.Width/32.0f), (OSInput.WindowSize.Height/18.0f)) / 0.5f;
    
    CollisionSystemNewFrame();
    
    EntityManager.UpdateAndRenderEntities(&RenderGroup);
    // NOTE(Tyler): Exit
    {
        v2 P = v2{15.25f, 3.25f};
        v2 DrawP = P-CameraP;
        v2 Radius = {0.25f, 0.25f};
        RenderRectangle(&RenderGroup, DrawP-Radius, DrawP+Radius, 0.0f, GREEN);
        collision_boundary *Boundary = &EntityManager.Player->Boundaries[0];
        v2 PlayerMin = EntityManager.Player->P-(Boundary->Size/2);
        v2 PlayerMax = EntityManager.Player->P+(Boundary->Size/2);
        if((P.X-Radius.X <= PlayerMax.X) &&
           (PlayerMin.X  <= P.X+Radius.X) &&
           (P.Y-Radius.Y <= PlayerMax.Y) &&
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
                RenderCenteredString(&RenderGroup, &MainFont, GREEN, TopCenter, -0.9f,
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
        RenderFormatString(&RenderGroup, &MainFont, Color, 
                           TopCenter.X-(0.5f*Advance), TopCenter.Y, -0.9f,
                           "Level completed!");
        
        CompletionCooldown -= OSInput.dTimeForFrame;
        if(CompletionCooldown < 0.00001f){
            CurrentWorld->Flags |= WorldFlag_IsCompleted;
            CompletionCooldown = 0.0f;
            ChangeState(GameMode_MainGame, "Overworld");
        }
    }
    
    // NOTE(Tyler): Weapon charge bar
    {
        v2 Min = v2{0.1f, 0.1f};
        v2 Max = Min;
        f32 Percent = 0.0f;
        Percent = EntityManager.Player->WeaponChargeTime;
        Max.X += 4.0f*Percent;
        Max.Y += 0.2f;
        RenderRectangle(&RenderGroup, Min, Max, -1.0f, color{1.0f, 0.0f, 1.0f, 0.9f});
    }
    
    // NOTE(Tyler): Health
    {
        f32 WindowWidth = OSInput.WindowSize.X / RenderGroup.MetersToPixels;
        v2 P = v2{0.2f, 0.8f};
        f32 XAdvance = 0.3f;
        
        player_entity *Player = EntityManager.Player;
        u32 FullHearts = Player->Health / 3;
        u32 Remainder = Player->Health % 3;
        
        Assert(FullHearts <= 3);
        u32 I;
        for(I = 0; I < FullHearts; I++){
            RenderFrameOfSpriteSheet(&RenderGroup, "heart", 0, P, -0.9f);
            P.X += XAdvance;
        }
        
        if(Remainder > 0){
            Remainder = 3 - Remainder;
            RenderFrameOfSpriteSheet(&RenderGroup, "heart", Remainder, P, -0.9f);
            P.X += XAdvance;
            I++;
        }
        
        if(I < 3){
            for(u32 J = 0; J < 3-I; J++){
                RenderFrameOfSpriteSheet(&RenderGroup, "heart", 3, P, -0.9f);
                P.X += XAdvance;
            }
        }
        
        RenderFormatString(&RenderGroup, &DebugFont, BLACK, 
                           RenderGroup.MetersToPixels*P.X, RenderGroup.MetersToPixels*P.Y,
                           -2.0f, "Health: %d", Player->Health);
    }
    
    //~ Debug UI
    layout Layout = CreateLayout(&RenderGroup, 100, OSInput.WindowSize.Height-100,
                                 30, DebugFont.Size, 100, -0.9f);
    LayoutString(&Layout, &MainFont,
                 GREEN, "Score: %u", Score);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "Counter: %.2f", Counter);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "TransientMemory:  %'jd", TransientStorageArena.Used);
    LayoutString(&Layout, &DebugFont,
                 BLACK, "PermanentMemory:  %'jd", PermanentStorageArena.Used);
    
    LayoutString(&Layout, &DebugFont, BLACK,
                 "Enemy count: %u", EntityManager.EnemyCount);
    
    {
        layout Layout = CreateLayout(&RenderGroup, OSInput.WindowSize.Width-500, OSInput.WindowSize.Height-100,
                                     30, DebugFont.Size);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Current level: %s", CurrentWorld->Name);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Use up and down arrows to change levels");
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Use 'e' to open the editor");
    }
    
    LayoutFps(&Layout);
    
    Layout.CurrentP.X += Layout.Advance.X;
    LayoutString(&Layout, &DebugFont,
                 BLACK, "Player: vel: %.2f %.2f, state: %u %.2f %.2f",
                 EntityManager.Player->dP.X, EntityManager.Player->dP.Y,
                 EntityManager.Player->State,
                 EntityManager.Player->AnimationState,
                 EntityManager.Player->Cooldown);
    
    Layout.CurrentP.X -= Layout.Advance.X;
    
    DebugRenderAllProfileData(&RenderGroup, &Layout);
    
    RenderGroupToScreen(&RenderGroup);
}