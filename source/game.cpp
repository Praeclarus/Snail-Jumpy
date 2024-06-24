
internal void
MainGameDoFrame(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input, 
                entity_manager *Entities, world_manager *Worlds, font *MainFont,
                world_editor *WorldEditor, settings_state *Settings){
    TIMED_FUNCTION();
    
    DO_DEBUG_INFO();
    
    //~ 
    if(Input->KeyJustDown('E', KeyFlag_Control)) ToggleWorldEditor(WorldEditor);
    if(Input->KeyJustDown(KeyCode_Escape)) ChangeState(GameMode_Menu, MakeString(0));
    
    //~ 
    {
        Renderer->NewFrame(&GlobalTransientMemory, Input->WindowSize, 
                           HSBToRGB(CurrentWorld->BackgroundColor), Input->dTime);
    }
    Renderer->CalculateCameraBounds(CurrentWorld); 
    Renderer->SetCameraSettings(0.3f/Input->dTime);
    Renderer->SetLightingConditions(HSBToRGB(CurrentWorld->AmbientColor), CurrentWorld->Exposure);
    
    render_group *GameGroup   = Renderer->GetRenderGroup(RenderGroupID_Lighting);
    render_group *GameUIGroup = Renderer->GetRenderGroup(RenderGroupID_NoLighting);
    Entities->UpdateEntities(Renderer, Assets, Mixer, Input, Settings);
    Entities->RenderEntities(GameGroup, Assets, Renderer, Input->dTime, Worlds);
    
    player_entity *Player = Entities->Player;
    
    if(CompletionCooldown > 0.0f){
        asset_font *Font = AssetsFind(Assets, Font, font_basic_bold);
        v2 Advance =
            FontStringAdvance(Font, 
                              "Level completed!");
        v2 Center = Renderer->ScreenToWorld(0.5f*Input->WindowSize, 0);
        color Color = GREEN;
        if(CompletionCooldown > 0.5f*3.0f){
            Color.A = 2.0f*(1.0f - CompletionCooldown/3.0f);
        }else if(CompletionCooldown < 0.3f*3.0f){
            Color.A = 2.0f * CompletionCooldown/3.0f;
        }
        FontRenderString(GameUIGroup, Font, Center, ZLayer(ZLayer_GameUI), Color, "Level completed!");
        
        CompletionCooldown -= Input->dTime;
        if(CompletionCooldown < 0.00001f){
            CurrentWorld->Flags |= WorldFlag_IsCompleted;
            CompletionCooldown = 0.0f;
            //ChangeState(GameMode_MainGame, "Overworld");
        }
    }
    
    //~ Weapon charge bar
    {
        v2 Min = V2(4.0f, 4.0f);
        v2 Max = Min;
        f32 Percent = 0.0f;
        Percent = Entities->Player->WeaponChargeTime;
        Max.X += 70.0f*Percent;
        Max.Y += 5.0f;
        RenderRect(GameUIGroup, MakeRect(Min, Max), ZLayer(0, ZLayer_GameUI), MakeColor(1.0f, 0.0f, 1.0f, 0.9f));
    }
    
    //~ Health display @render_health_display
    {
        f32 XAdvance = 10.0f;
        f32 Margin   = 5.0f;
        
        s32 MaxHealth = Player->MaxHealth;
        f32 Width = XAdvance*MaxHealth + Margin;
        f32 Height = 10 + Margin;
        
        v2 TopRight = Renderer->ScreenToWorld(Renderer->OutputSize, 0)-V2(Width, Height);
        v2 P = V2(Margin, TopRight.Y);
        
        Player->VisualHealth = Clamp(Player->VisualHealth, 0, (3*MaxHealth));
        u32 Health = (u32)Player->VisualHealth;
        
        asset_sprite_sheet *Asset = AssetsFind(Assets, SpriteSheet, heart);
        for(s32 I = 0; I < MaxHealth; I++){
            u32 Frame = 3;
            if(Health > 3){
                Health -= 3;
                Frame = 0;
            }else if(Health > 0){
                Frame = 3-Health;
                Health = 0;
            }
            RenderSpriteSheetAnimationFrame(GameUIGroup, Asset, P, ZLayer(0, ZLayer_GameUI), 1, Frame);
            P.X += XAdvance;
        }
        
        f32 Threshold = 0.05f;
        if(3*Player->Health-Player->VisualHealth != 0){
            Player->VisualHealthUpdateT += Input->dTime;
        }
        if(Player->VisualHealthUpdateT > Threshold){
            Player->VisualHealth += SignOf(3*Player->Health - Player->VisualHealth);
            Player->VisualHealthUpdateT = 0;
        }
        
#if 0        
        if(Remainder > 0){
            Remainder = 3 - Remainder;
            RenderSpriteSheetAnimationFrame(GameUIGroup, Asset, P, ZLayer(0, ZLayer_GameUI), 1, Remainder);
            P.X += XAdvance;
            I++;
        }
        
        for(u32 I=0; I<EmptyHearts; I++){
            RenderSpriteSheetAnimationFrame(GameUIGroup, Asset, P, ZLayer(0, ZLayer_GameUI), 1, 3);
            P.X += XAdvance;
        }
#endif
    }
    
#if 0    
    {
        v2 WindowSize = Renderer->ScreenToWorld(Input->WindowSize, 0);
        asset_font *Font = AssetsFind(Assets, Font, font_basic);
        FontRenderString(GameUIGroup, Font, V2(10, WindowSize.Y-10-Font->Height), ZLayer(ZLayer_GameUI), WHITE, "Score: %u", Score);
    }
#endif
}