// TODO(Tyler): This also logs all the data
internal void
DEBUGRenderAllProfileData(render_group *RenderGroup, layout *Layout){
    for(u32 ProfileIndex = 0;
        ProfileIndex < ProfileData.CurrentBlockIndex+1;
        ProfileIndex++){
        profiled_block *Block = &ProfileData.Blocks[ProfileIndex];
        if(Block->Name == 0){
            continue;
        }
        
        f32 ActualX = Layout->CurrentP.X + (Layout->Advance.X*(f32)Block->Level);
        RenderFormatString(RenderGroup, &DebugFont, BLACK,
                           ActualX, Layout->CurrentP.Y, -1.0f,
                           "%s: %'8llucy", Block->Name, Block->CycleCount);
        Layout->CurrentP.Y -= DebugFont.Size;
    }
}

internal void
DEBUGRenderOverlay(render_group *RenderGroup){
    layout Layout = CreateLayout(RenderGroup, 100, OSInput.WindowSize.Height-150, 30, DebugFont.Size, 100, -0.9f);
    if(DebugConfig.Overlay & DebugOverlay_Miscellaneous){
        LayoutString(&Layout, &DebugFont,
                     BLACK, "Counter: %.2f", Counter);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "TransientMemory:  %'jd", TransientStorageArena.Used);
        LayoutString(&Layout, &DebugFont,
                     BLACK, "PermanentMemory:  %'jd", PermanentStorageArena.Used);
        
    }
    if(DebugConfig.Overlay & DebugOverlay_Profiler){
        LayoutFps(&Layout);
        DEBUGRenderAllProfileData(RenderGroup, &Layout);
    }
}
