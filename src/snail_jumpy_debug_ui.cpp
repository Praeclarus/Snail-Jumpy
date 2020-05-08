// TODO(Tyler): This also logs all the data
internal void
DebugRenderAllProfileData(render_group *RenderGroup, layout *Layout){
    for(u32 ProfileIndex = 0;
        ProfileIndex < GlobalProfileData.CurrentBlockIndex+1;
        ProfileIndex++){
        profiled_block *Block = &GlobalProfileData.Blocks[ProfileIndex];
        if(Block->Name == 0){
            continue;
        }
        
        f32 ActualX = Layout->CurrentP.X + (Layout->Advance.X*(f32)Block->Level);
        RenderFormatString(RenderGroup, &GlobalDebugFont, BLACK,
                           ActualX, Layout->CurrentP.Y, -1.0f,
                           "%s: %'8llucy", Block->Name, Block->CycleCount);
        Layout->CurrentP.Y -= GlobalDebugFont.Size;
    }
}