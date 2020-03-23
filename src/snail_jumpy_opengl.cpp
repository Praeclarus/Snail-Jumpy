
internal void
RenderGroupWithOpenGl(game_state *GameState, render_group *RenderGroup, v2 WindowSize){
    
#if 0
    glBindTexture(GL_TEXTURE_2D, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Buffer->Width, Buffer->Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Buffer->Memory);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif
    
    glViewport(0, 0, (GLsizei)WindowSize.Width, (GLsizei)WindowSize.Height);
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glMatrixMode(GL_PROJECTION);
    f32 A = 2.0f/((f32)WindowSize.Width/GameState->MetersToPixels);
    f32 B = 2.0f/((f32)WindowSize.Height/GameState->MetersToPixels);
    f32 Projection[] = {
        A,   0, 0, 0,
        0,   B, 0, 0,
        0,   0, 1, 0,
        -1, -1, 0, 1,
    };
    glLoadMatrixf(Projection);
    
    glEnable(GL_TEXTURE_2D);
    
    for(u32 Index = 0; Index < RenderGroup->Count; Index++){
        glBegin(GL_TRIANGLES);
        
        render_group_item *RenderItem = &RenderGroup->Items[Index];
        switch(RenderItem->Type){
            case RenderItemType_Rectangle:
            {
                glColor3f((f32)((RenderItem->Color&0x00FF0000)>>16)/255.0f,
                          (f32)((RenderItem->Color&0x0000FF00)>>8)/255.0f,
                          (f32)((RenderItem->Color&0x000000FF)>>0)/255.0f);
                
                glVertex2f(RenderItem->MinCorner.X, RenderItem->MinCorner.Y);
                glVertex2f(RenderItem->MinCorner.X, RenderItem->MaxCorner.Y);
                glVertex2f(RenderItem->MaxCorner.X, RenderItem->MaxCorner.Y);
                
                glVertex2f(RenderItem->MinCorner.X, RenderItem->MinCorner.Y);
                glVertex2f(RenderItem->MaxCorner.X, RenderItem->MinCorner.Y);
                glVertex2f(RenderItem->MaxCorner.X, RenderItem->MaxCorner.Y);
            }break;
            
        }
        glEnd();
    }
}
