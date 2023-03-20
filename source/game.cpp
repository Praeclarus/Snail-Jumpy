
internal void
MainGameDoFrame(game_renderer *Renderer, audio_mixer *Mixer, asset_system *Assets, os_input *Input, 
                entity_manager *Entities, world_manager *Worlds, font *MainFont,
                world_editor *WorldEditor, settings_state *Settings){
    TIMED_FUNCTION();
    //~ 
    if(Input->KeyJustDown('E', KeyFlag_Control)) ToggleWorldEditor(WorldEditor);
    if(Input->KeyJustDown(KeyCode_Escape)) ChangeState(GameMode_Menu, MakeString(0));
    
    //~ 
    Renderer->NewFrame(&GlobalTransientMemory, Input->WindowSize, MakeColor(0.30f, 0.55f, 0.70f), Input->dTime);
    Renderer->CalculateCameraBounds(CurrentWorld); 
    Renderer->SetCameraSettings(0.3f/Input->dTime);
    Renderer->SetLightingConditions(HSBToRGB(CurrentWorld->AmbientColor), CurrentWorld->Exposure);
    
    
    render_group *GameGroup       = Renderer->GetRenderGroup(RenderGroupID_Lighting);
    render_group *NoLightingGroup = Renderer->GetRenderGroup(RenderGroupID_NoLighting);
    Entities->UpdateEntities(Renderer, Assets, Input, Settings);
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
        FontRenderString(NoLightingGroup, Font, Center, ZLayer(ZLayer_GameUI), Color, "Level completed!");
        
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
        RenderRect(GameGroup, MakeRect(Min, Max), ZLayer(0, ZLayer_GameUI), MakeColor(1.0f, 0.0f, 1.0f, 0.9f));
    }
    
    //~ Health display
    {
        v2 P = V2(10.0f, 10.0f);
        f32 XAdvance = 10.0f;
        
        u32 FullHearts = Player->Health / 3;
        u32 Remainder = Player->Health % 3;
        
        asset_sprite_sheet *Asset = AssetsFind(Assets, SpriteSheet, heart);
        Assert(FullHearts <= 3);
        u32 I;
        for(I = 0; I < FullHearts; I++){
            RenderSpriteSheetAnimationFrame(GameGroup, Asset, P, ZLayer(0, ZLayer_GameUI), 1, 0);
            P.X += XAdvance;
        }
        
        if(Remainder > 0){
            Remainder = 3 - Remainder;
            RenderSpriteSheetAnimationFrame(GameGroup, Asset, P, ZLayer(0, ZLayer_GameUI), 1, Remainder);
            P.X += XAdvance;
            I++;
        }
        
        if(I < 3){
            for(u32 J = 0; J < 3-I; J++){
                RenderSpriteSheetAnimationFrame(GameGroup, Asset, P, ZLayer(0, ZLayer_GameUI) , 1, 3);
                P.X += XAdvance;
            }
        }
    }
    
#if 0
    //~ Rope/vine thing
    {
        v2 BaseP = V2(100, 110);
        
        f32 FinalT = (0.5f*Sin(2*Counter))+0.5f;
        f32 MinAngle = 0.4*PI;
        f32 MaxAngle = 0.6f*PI;
        f32 Angle = Lerp(MinAngle, MaxAngle, FinalT);
        v2 Delta = 50.0f*V2(Cos(Angle), -Sin(Angle));
        
        RenderLineFrom(BaseP, Delta, 0.0f, 1.0f, GREEN, GameItem(1));
        GameRenderer.AddLight(BaseP+Delta, MakeColor(0.0f, 1.0f, 0.0f), 0.3f, 5.0f, GameItem(1));
    }
#endif
    
    {
        v2 WindowSize = Renderer->ScreenToWorld(Input->WindowSize, 0);
        asset_font *Font = AssetsFind(Assets, Font, font_basic);
        FontRenderString(NoLightingGroup, Font, V2(10, WindowSize.Y-10-Font->Height), ZLayer(ZLayer_GameUI), WHITE, "Score: %u", Score);
    }
}