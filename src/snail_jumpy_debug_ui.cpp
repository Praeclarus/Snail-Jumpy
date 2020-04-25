internal void
RenderAllProfileData(temporary_memory *RenderMemory, render_group *RenderGroup,
                     f32 X, f32 *Y, f32 XAdvance, f32 YAdvance){
    for(u32 ProfileIndex = 0;
        ProfileIndex < ArrayCount(GlobalProfileData.Blocks);
        ProfileIndex++){
        profiled_block *Block = &GlobalProfileData.Blocks[ProfileIndex];
        if(Block->Name == 0){
            continue;
        }
        
        f32 ActualX = X + (XAdvance*(f32)Block->Level);
        RenderFormatString(RenderMemory, RenderGroup, &GlobalDebugFont,
                           {0.0f, 0.0f, 0.0f, 1.0f},
                           ActualX, *Y, 0.0f,
                           "%s: %'llucy", Block->Name, Block->CycleCount);
        *Y -= YAdvance;
    }
}