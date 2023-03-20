
//~ Basic rendering primitives

internal inline b8
ColorHasAlpha(color C){
    b8 Result = (C.A != 0.0f) && (C.A < 1.0f);
    return(Result);
}

internal void
RenderQuad(render_group *Group, render_texture Texture, z_layer Z,
           v2 P0, v2 T0, color C0, 
           v2 P1, v2 T1, color C1, 
           v2 P2, v2 T2, color C2, 
           v2 P3, v2 T3, color C3,
           b8 HasAlpha_=false){
    b8 HasAlpha = (HasAlpha_         ||
                   ColorHasAlpha(C0) ||
                   ColorHasAlpha(C1) ||
                   ColorHasAlpha(C2) ||
                   ColorHasAlpha(C3));
    game_renderer *Renderer = Group->Renderer;
    render_item *RenderItem = Renderer->NewRenderItem(Group, Texture, HasAlpha, Z);
    
    basic_vertex *Vertices = Renderer->AddVertices(RenderItem, 4);
    Vertices[0] = {P0, (f32)Z.Z, T0, C0};
    Vertices[1] = {P1, (f32)Z.Z, T1, C1};
    Vertices[2] = {P2, (f32)Z.Z, T2, C2};
    Vertices[3] = {P3, (f32)Z.Z, T3, C3};
    
    Renderer->DoParallax(RenderItem, Z.Layer);
    
    u32 *Indices = Renderer->AddIndices(RenderItem, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderLine(render_group *Group, v2 A, v2 B, z_layer Z, f32 Thickness, color Color){
    v2 AB = B - A;
    v2 AB90 = V2Normalize(V2Clockwise90(AB));
    v2 P0 = A - 0.5f*Thickness*AB90;
    v2 P1 = A + 0.5f*Thickness*AB90;
    v2 P3 = B - 0.5f*Thickness*AB90;
    v2 P2 = B + 0.5f*Thickness*AB90;
    game_renderer *Renderer = Group->Renderer;
    RenderQuad(Group, Renderer->WhiteTexture, Z,
               P0, V2(0, 0), Color,
               P1, V2(0, 1), Color,
               P2, V2(1, 1), Color,
               P3, V2(1, 0), Color);
}

internal void
RenderLineFrom(render_group *Group, v2 A, v2 Delta, z_layer Z, f32 Thickness, color Color){
    RenderLine(Group, A, A+Delta, Z, Thickness, Color);
}

internal void
RenderCircle(render_group *Group, v2 P, f32 Radius, z_layer Z, color Color,
             u32 Sides=30){
    
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Sides;
    game_renderer *Renderer = Group->Renderer;
    render_item *RenderItem = Renderer->NewRenderItem(Group, Renderer->WhiteTexture, ColorHasAlpha(Color), Z);
    
    basic_vertex *Vertices = Renderer->AddVertices(RenderItem, Sides+2);
    Vertices[0] = {P, (f32)Z.Z, 0.0f, 0.0f, Color};
    for(u32 I = 0; I <= Sides; I++){
        v2 Offset = V2(Cos(T*TAU), Sin(T*TAU));
        Vertices[I+1] = {P+Radius*Offset, (f32)Z.Z, V2(0.0f, 0.0f), Color};
        T += Step;
    }
    
    u32 *Indices = Renderer->AddIndices(RenderItem, Sides*3);
    u16 CurrentIndex = 1;
    for(u32 I = 0; I < Sides*3; I += 3){
        Indices[I] = 0;
        Indices[I+1] = CurrentIndex;
        CurrentIndex++;
        if(CurrentIndex == Sides+1){
            CurrentIndex = 1;
        }
        Indices[I+2] = CurrentIndex;
    }
    
}

internal void
RenderRect(render_group *Group, rect R, z_layer Z, color Color){
    RenderQuad(Group, Group->Renderer->WhiteTexture, Z,
               V2(R.Min.X, R.Min.Y), V2(0, 0), Color,
               V2(R.Min.X, R.Max.Y), V2(0, 1), Color,
               V2(R.Max.X, R.Max.Y), V2(1, 1), Color,
               V2(R.Max.X, R.Min.Y), V2(1, 0), Color);
}

internal inline void
RenderRectOutline(render_group *Group, rect R, z_layer Z, color Color, f32 Thickness=1){
    rect B = RectGrow(R, Thickness);
    RenderRect(Group, MakeRect(V2(B.Min.X, B.Min.Y), V2(R.Max.X, R.Min.Y)), Z, Color);
    RenderRect(Group, MakeRect(V2(R.Max.X, B.Min.Y), V2(B.Max.X, R.Max.Y)), Z, Color);
    RenderRect(Group, MakeRect(V2(R.Min.X, R.Max.Y), V2(B.Max.X, B.Max.Y)), Z, Color);
    RenderRect(Group, MakeRect(V2(B.Min.X, R.Min.Y), V2(R.Min.X, B.Max.Y)), Z, Color);
    //RenderRect(Rect(V2(R.Min.X, R.Min.Y),           V2(R.Max.X, R.Min.Y+Thickness)), Z, Color);
    //RenderRect(Rect(V2(R.Max.X-Thickness, R.Min.Y), V2(R.Max.X, R.Max.Y)),           Z, Color);
    //RenderRect(Rect(V2(R.Min.X, R.Max.Y),           V2(R.Max.X, R.Max.Y-Thickness)), Z, Color);
    //RenderRect(Rect(V2(R.Min.X, R.Min.Y),           V2(R.Min.X+Thickness, R.Max.Y)), Z, Color);
}

internal void
RenderTexture(render_group *Group, rect R, z_layer Z, render_texture Texture, 
              rect TextureRect=MakeRect(V2(0,0), V2(1,1)), b8 HasAlpha=false, color Color = WHITE){
    Assert(Texture);
    
    RenderQuad(Group, Texture, Z,
               V2(R.Min.X, R.Min.Y), V2(TextureRect.Min.X, TextureRect.Min.Y), Color,
               V2(R.Min.X, R.Max.Y), V2(TextureRect.Min.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Max.Y), V2(TextureRect.Max.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Min.Y), V2(TextureRect.Max.X, TextureRect.Min.Y), Color,
               HasAlpha);
}

internal void
RenderRoundedRect(render_group *Group, rect R, z_layer Z, f32 Roundness, color C, 
                  rounded_rect_corner Corners=RoundedRectCorner_All){
    game_renderer *Renderer = Group->Renderer;
    R = RectRectify(R);
    v2 Size = RectSize(R);
    
    f32 Radius;
    if(Roundness < 0){
        Radius = 0.5f*Minimum(Size.Width, Size.Height) * -Roundness;
    }else{
        Radius = Roundness;
    }
    
    b8 HasAlpha = ColorHasAlpha(C);
    
    u32 Segments = (u32)Round(TAU*Radius / 5.0f);
    
    v2 R0 = V2(R.Min.X, R.Max.Y);
    v2 R1 = V2(R.Max.X, R.Max.Y);
    v2 R2 = V2(R.Max.X, R.Min.Y);
    v2 R3 = V2(R.Min.X, R.Min.Y);
    
    v2 I0 = R0 + V2( Radius, -Radius);
    v2 I1 = R1 + V2(-Radius, -Radius);
    v2 I2 = R2 + V2(-Radius,  Radius);
    v2 I3 = R3 + V2( Radius,  Radius);
    
    v2 O0 = R0 + V2( Radius,       0);
    v2 O1 = R1 + V2(-Radius,       0);
    v2 O2 = R1 + V2(      0, -Radius);
    v2 O3 = R2 + V2(      0,  Radius);
    v2 O4 = R2 + V2(-Radius,       0);
    v2 O5 = R3 + V2( Radius,       0);
    v2 O6 = R3 + V2(      0,  Radius);
    v2 O7 = R0 + V2(      0, -Radius);
    
    u32 VertexCount = 12;
    u32 IndexCount  = 6*5;
    
    // Offsets
    u32 VO0 = VertexCount;
    u32 IO0 = IndexCount;
    if(Corners & RoundedRectCorner_TopLeft){
        VertexCount += 2*Segments;
        IndexCount  += 3*Segments;
    }else{
        VertexCount += 1;
        IndexCount += 6;
    }
    
    u32 VO1 = VertexCount;
    u32 IO1 = IndexCount;
    if(Corners & RoundedRectCorner_TopRight){
        VertexCount += 2*Segments;
        IndexCount  += 3*Segments;
    }else{
        VertexCount += 1;
        IndexCount += 6;
    }
    
    u32 VO2 = VertexCount;
    u32 IO2 = IndexCount;
    if(Corners & RoundedRectCorner_BottomRight){
        VertexCount += 2*Segments;
        IndexCount  += 3*Segments;
    }else{
        VertexCount += 1;
        IndexCount += 6;
    }
    
    u32 VO3 = VertexCount;
    u32 IO3 = IndexCount;
    if(Corners & RoundedRectCorner_BottomLeft){
        VertexCount += 2*Segments;
        IndexCount  += 3*Segments;
    }else{
        VertexCount += 1;
        IndexCount += 6;
    }
    
    render_item *Item = Renderer->NewRenderItem(Group, Renderer->WhiteTexture, HasAlpha, Z);
    basic_vertex *Vertices = Renderer->AddVertices(Item, VertexCount);
    u32 *Indices = Renderer->AddIndices(Item, IndexCount);
    
    Vertices[ 0] = {I0, (f32)Z.Z, V2(0, 1), C};
    Vertices[ 1] = {I1, (f32)Z.Z, V2(1, 1), C};
    Vertices[ 2] = {I2, (f32)Z.Z, V2(1, 0), C};
    Vertices[ 3] = {I3, (f32)Z.Z, V2(0, 0), C};
    Vertices[ 4] = {O0, (f32)Z.Z, V2(0, 1), C};
    Vertices[ 5] = {O1, (f32)Z.Z, V2(1, 1), C};
    Vertices[ 6] = {O2, (f32)Z.Z, V2(1, 1), C};
    Vertices[ 7] = {O3, (f32)Z.Z, V2(1, 0), C};
    Vertices[ 8] = {O4, (f32)Z.Z, V2(1, 0), C};
    Vertices[ 9] = {O5, (f32)Z.Z, V2(0, 0), C};
    Vertices[10] = {O6, (f32)Z.Z, V2(0, 0), C};
    Vertices[11] = {O7, (f32)Z.Z, V2(0, 1), C};
    
    // Inner rect
    Indices[ 0] = 0;
    Indices[ 1] = 1;
    Indices[ 2] = 2;
    Indices[ 3] = 0;
    Indices[ 4] = 2;
    Indices[ 5] = 3;
    
    // Top rect
    Indices[ 6] = 4;
    Indices[ 7] = 5;
    Indices[ 8] = 1;
    Indices[ 9] = 4;
    Indices[10] = 1;
    Indices[11] = 0;
    
    // Right rect
    Indices[12] = 1;
    Indices[13] = 6;
    Indices[14] = 7;
    Indices[15] = 1;
    Indices[16] = 7;
    Indices[17] = 2;
    
    // Bottom rect
    Indices[18] = 3;
    Indices[19] = 2;
    Indices[20] = 8;
    Indices[21] = 3;
    Indices[22] = 8;
    Indices[23] = 9;
    
    // Left rect
    Indices[24] = 11;
    Indices[25] =  0;
    Indices[26] =  3;
    Indices[27] = 11;
    Indices[28] =  3;
    Indices[29] = 10;
    
    if(!(Corners & RoundedRectCorner_TopLeft)){
        Vertices[VO0] = {R0, (f32)Z.Z, V2(0.0f, 0.0f), C};
        Indices[IO0++] = VO0;
        Indices[IO0++] = 4;
        Indices[IO0++] = 0;
        Indices[IO0++] = VO0;
        Indices[IO0++] = 0;
        Indices[IO0++] = 11;
    }
    
    if(!(Corners & RoundedRectCorner_TopRight)){
        Vertices[VO1] = {R1, (f32)Z.Z, V2(0.0f, 0.0f), C};
        Indices[IO1++] = 5;
        Indices[IO1++] = VO1;
        Indices[IO1++] = 6;
        Indices[IO1++] = 5;
        Indices[IO1++] = 6;
        Indices[IO1++] = 1;
    }
    
    if(!(Corners & RoundedRectCorner_BottomRight)){
        Vertices[VO2] = {R2, (f32)Z.Z, V2(0.0f, 0.0f), C};
        Indices[IO2++] = 2;
        Indices[IO2++] = 7;
        Indices[IO2++] = VO2;
        Indices[IO2++] = 2;
        Indices[IO2++] = VO2;
        Indices[IO2++] = 8;
    }
    
    if(!(Corners & RoundedRectCorner_BottomLeft)){
        Vertices[VO3] = {R3, (f32)Z.Z, V2(0.0f, 0.0f), C};
        Indices[IO3++] = 10;
        Indices[IO3++] =  3;
        Indices[IO3++] =  9;
        Indices[IO3++] = 10;
        Indices[IO3++] =  9;
        Indices[IO3++] = VO3;
    }
    
    f32 T = 0.0f;
    f32 Step = 0.25f * 1.0f/(f32)Segments;
    for(u32 I=0; I<Segments; I++){
        f32 T0 = T;
        T += Step;
        f32 T1 = T;
        
        f32 C0 = Cos(T0*TAU);
        f32 S0 = Sin(T0*TAU);
        f32 C1 = Cos(T1*TAU);
        f32 S1 = Sin(T1*TAU);
        
        if(Corners & RoundedRectCorner_TopLeft){
            Indices[IO0++] = 0;
            Vertices[VO0] = {I0+Radius*V2(-C0, S0), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO0++] = VO0++;
            Vertices[VO0] = {I0+Radius*V2(-C1, S1), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO0++] = VO0++;
        }
        
        if(Corners & RoundedRectCorner_TopRight){
            Indices[IO1++] = 1;
            Vertices[VO1] = {I1+Radius*V2(C0, S0), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO1++] = VO1++;
            Vertices[VO1] = {I1+Radius*V2(C1, S1), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO1++] = VO1++;
        }
        
        if(Corners & RoundedRectCorner_BottomRight){
            Indices[IO2++] = 2;
            Vertices[VO2] = {I2+Radius*V2(C0, -S0), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO2++] = VO2++;
            Vertices[VO2] = {I2+Radius*V2(C1, -S1), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO2++] = VO2++;
        }
        
        if(Corners & RoundedRectCorner_BottomLeft){
            Indices[IO3++] = 3;
            Vertices[VO3] = {I3+Radius*V2(-C0, -S0), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO3++] = VO3++;
            Vertices[VO3] = {I3+Radius*V2(-C1, -S1), (f32)Z.Z, V2(0.0f, 0.0f), C};
            Indices[IO3++] = VO3++;
        }
    }
    
    Renderer->DoParallax(Item, Z.Layer);
}

//~ String rendering

internal void
RenderString(render_group *Group, font *Font, color Color, v2 P, z_layer Z, const char *String){
    game_renderer *Renderer = Group->Renderer;
    v2 OutputSize = Renderer->OutputSize;
    
    P.Y = OutputSize.Y - P.Y;
    
    u32 Length = CStringLength(String);
    
    render_item *RenderItem = Renderer->NewRenderItem(Group, Font->Texture, true, Z);
    
    basic_vertex *Vertices = Renderer->AddVertices(RenderItem, 4*Length);
    u32 VertexOffset = 0;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &P.X, &P.Y, &Q, 1);
        Q.y0 = OutputSize.Y - Q.y0;
        Q.y1 = OutputSize.Y - Q.y1;
        Vertices[VertexOffset]   = {V2(Q.x0, Q.y0), (f32)Z.Z, V2(Q.s0, Q.t0), Color};
        Vertices[VertexOffset+1] = {V2(Q.x0, Q.y1), (f32)Z.Z, V2(Q.s0, Q.t1), Color};
        Vertices[VertexOffset+2] = {V2(Q.x1, Q.y1), (f32)Z.Z, V2(Q.s1, Q.t1), Color};
        Vertices[VertexOffset+3] = {V2(Q.x1, Q.y0), (f32)Z.Z, V2(Q.s1, Q.t0), Color};
        
        VertexOffset += 4;
    }
    
    u32 *Indices = Renderer->AddIndices(RenderItem, 6*Length);
    u16 FaceOffset = 0;
    for(u32 IndexOffset = 0; IndexOffset < 6*Length; IndexOffset += 6){
        Indices[IndexOffset]   = FaceOffset;
        Indices[IndexOffset+1] = FaceOffset+1;
        Indices[IndexOffset+2] = FaceOffset+2;
        Indices[IndexOffset+3] = FaceOffset;
        Indices[IndexOffset+4] = FaceOffset+2;
        Indices[IndexOffset+5] = FaceOffset+3;
        FaceOffset += 4;
    }
}

internal inline void
VRenderFormatString(render_group *Group, font *Font, color Color, 
                    v2 P, z_layer Z, const char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    RenderString(Group, Font, Color, P, Z, Buffer);
}

internal inline void
RenderFormatString(render_group *Group, font *Font, color Color, v2 P, z_layer Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Group, Font, Color, P, Z, Format, VarArgs);
    va_end(VarArgs);
}

internal f32
GetStringAdvance(font *Font, const char *String, b8 CountEndingSpace=false){
    f32 Result = 0;
    f32 X = 0.0f;
    f32 Y = 0.0f;
    char LastC = '\0';
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Result = Q.x1;
        LastC = C;
    }
    
    // NOTE(Tyler): This is so that the ending space is actually factored in, usually it is
    // not.
    if(CountEndingSpace){
        if(LastC == ' '){
            stbtt_aligned_quad Q;
            stbtt_GetBakedQuad(Font->CharData,
                               Font->TextureWidth, Font->TextureHeight,
                               LastC-32, &X, &Y, &Q, 1);
            Result = Q.x1;
        }
    }
    
    return(Result);
}

internal f32
GetStringAdvanceByCount(font *Font, const char *String, u32 Count, b8 CountEndingSpace=false){
    f32 Result = 0;
    f32 X = 0.0f;
    f32 Y = 0.0f;
    char LastC = '\0';
    u32 I = 0;
    for(char C = *String; C; C = *(++String)){
        if(I >= Count) break;
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &X, &Y, &Q, 1);
        Result = Q.x1;
        LastC = C;
        I++;
    }
    
    // NOTE(Tyler): This is so that the ending space is actually factored in, usually it is
    // not.
    if(CountEndingSpace){
        if(LastC == ' '){
            stbtt_aligned_quad Q;
            stbtt_GetBakedQuad(Font->CharData,
                               Font->TextureWidth, Font->TextureHeight,
                               LastC-32, &X, &Y, &Q, 1);
            Result = Q.x1;
        }
    }
    
    return(Result);
}

struct string_sizing_context {
    font *Font;
    const char *String;
    f32 X, Y;
};

internal inline string_sizing_context
StringSizingBegin(font *Font, const char *String){
    string_sizing_context Result = {};
    Result.Font = Font;
    Result.String = String;
    return Result;
}

internal inline f32
StringSizingStep(string_sizing_context *Context){
    font *Font = Context->Font;
    stbtt_aligned_quad Q;
    stbtt_GetBakedQuad(Font->CharData,
                       Font->TextureWidth, Font->TextureHeight,
                       *Context->String-32, &Context->X, &Context->Y, &Q, 1);
    if(*(Context->String+1)) Context->String++;
    return Q.x1;
}

internal inline f32
VGetFormatStringAdvance(font *Font, const char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    f32 Result = GetStringAdvance(Font, Buffer);
    return(Result);
}

internal inline f32
GetFormatStringAdvance(font *Font, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Result = VGetFormatStringAdvance(Font, Format, VarArgs);
    va_end(VarArgs);
    return(Result);
}

internal inline void 
RenderCenteredString(render_group *Group, font *Font, color Color, v2 Center,
                     z_layer Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Advance = VGetFormatStringAdvance(Font, Format, VarArgs);
    Center.X -= Advance/2.0f;
    VRenderFormatString(Group, Font, Color, Center, Z, Format, VarArgs);
    va_end(VarArgs);
}

//~ Z sorting
// NOTE(Tyler): Bottom-up merge sort implementation
// There is almost certainly a better sorting algorithm I could use, but this one will 
// work just fine for now. Its not terribly slow like insetion sorting, and likely some
// varient of this would be required for the current render_node scheme

// NOTE(Tyler): 'OutZs' has '2*SubCount' elements
internal void
MergeSortZsMerge(render_item_z *LeftZs, render_item_z *RightZs, 
                 u32 LeftCount, u32 RightCount,
                 render_item_z *OutZs){
    u32 LeftI  = 0;
    u32 RightI = 0;
    u32 TotalCount = LeftCount+RightCount;
    
    for(u32 I=0; I<TotalCount; I++){
        if((LeftI < LeftCount) && 
           ((RightI >= RightCount) || (LeftZs[LeftI].Z >= RightZs[RightI].Z))){
            // If all of the right array has been copied, 
            // or the left z is greater than or equal to the right z
            OutZs[I] = LeftZs[LeftI];
            LeftI++;
        }else{
            // If all of the left array has been copied, 
            // or the right z is greater than the left z
            OutZs[I] = RightZs[RightI];
            RightI++;
        }
    }
}

internal render_item_z *
MergeSortZs(render_item_z *ZsA, render_item_z *ZsB, u32 Count){
    // Double the width every iteration, until it is the size of the array
    for(u32 Width=1; Width<Count; Width = 2*Width){
        
        // Split the entire array into sub-arrays of 'Width' elements
        // and merge two of them
        for(u32 I=0; I<Count; I = I+2*Width){
            
            // Merge two sub-arrays of 'Width' elements
            u32 LeftArray  = I;
            u32 RightArray = Minimum(I+Width, Count);
            u32 End        = Minimum(I+2*Width, Count);
            
            MergeSortZsMerge(&ZsA[LeftArray], &ZsA[RightArray], 
                             RightArray-LeftArray, End-RightArray,
                             &ZsB[LeftArray]);
        }
        
        // Swap the two buffers
        render_item_z *Temp = ZsA;
        ZsA = ZsB;
        ZsB = Temp;
    }
    
    render_item_z *Result = ZsA;
    return(Result);
}

//~ Renderer
internal inline basic_shader
MakeBasicShaderFromData(entire_file File){
    basic_shader Result = {};
    Result.ID = MakeShaderProgramFromFileData(File);
    if(Result.ID == -1) return Result;
    
    Result.ProjectionLocation = ShaderProgramGetUniformLocation(Result.ID, "InProjection");
    
    return Result;
}

internal inline basic_shader
MakeBasicShaderFromPath(memory_arena *Memory, const char *Path){
    entire_file File = ReadEntireFile(Memory, Path);
    return MakeBasicShaderFromData(File);
}

void
game_renderer::Initialize(memory_arena *Arena, v2 OutputSize_){
    OutputSize = OutputSize_;
    ClipRects = MakeStack<rect>(Arena, MAX_CLIP_RECTS);
    
    //~ Camera
    CameraScale = 5;
    
    //~ Lights
    AmbientLight = 0.9f*MakeColor(0.25f, 0.2f, 0.2f, 1.0f);
    Exposure = 1.2f;
    
    //~ Other
    u8 TemplateColor[] = {0xff, 0xff, 0xff, 0xff};
    WhiteTexture = MakeTexture();
    TextureUpload(WhiteTexture, TemplateColor, 1, 1);
    
    entire_file FontShaderFile = ReadEntireFile(&GlobalTransientMemory, "shaders/glsl/font_shader.glsl");
    
    entire_file GameScreenShaderFile = ReadEntireFile(&GlobalTransientMemory, "shaders/glsl/game_screen_shader.glsl");
    GameScreenShader = MakeScreenShaderFromFileData(GameScreenShaderFile);
    InitializeFramebuffer(&GameScreenFramebuffer, OutputSize/CameraScale);
    
    entire_file DefaultShaderFile = ReadEntireFile(&GlobalTransientMemory, "shaders/glsl/default_shader.glsl");
    GameLightingShader   = MakeBasicShaderFromPath(&GlobalTransientMemory, "shaders/glsl/game_lighting_shader.glsl");
    ShaderProgramSetupLighting(GameLightingShader.ID);
    GameNoLightingShader = MakeBasicShaderFromPath(&GlobalTransientMemory, "shaders/glsl/game_no_lighting_shader.glsl");
    DefaultShader        = MakeBasicShaderFromData(DefaultShaderFile);
    ScaledShader         = MakeBasicShaderFromData(DefaultShaderFile);
    FontShader           = MakeBasicShaderFromPath(&GlobalTransientMemory, "shaders/glsl/font_shader.glsl");
    NoisyShader          = MakeBasicShaderFromPath(&GlobalTransientMemory, "shaders/glsl/game_noisy_shader.glsl");
    ShaderProgramSetupLighting(NoisyShader.ID);
    
    InitializeArray(&Vertices, 2000);
    InitializeArray(&Indices,  2000);
}

void
game_renderer::ChangeScale(f32 NewScale){
    f32 Epsilon = 0.0001f;
    if((NewScale <= (CameraScale+Epsilon)) &&((CameraScale-Epsilon) <= NewScale)){
    }else{
        CameraScale = NewScale;
        ResizeFramebuffer(&GameScreenFramebuffer, OutputSize/CameraScale);
        RenderGroups[RenderGroupID_Scaled].Scale = CameraScale;
    }
}

void
game_renderer::NewFrame(memory_arena *Arena, v2 OutputSize_, color ClearColor_, f32 dTime){
    v2 OldOutputSize = OutputSize;
    OutputSize = OutputSize_;
    ClearColor = ClearColor_;
    StackClear(&ClipRects);
    StackPush(&ClipRects, SizeRect(V2(0), OutputSize));
    
    //~ Render items
    RenderItemCount = 0;
    ArrayClear(&Vertices);
    ArrayClear(&Indices);
    
    for(u32 I=0; I<RenderType_TOTAL; I++){
        Nodes[I] = ArenaPushType(Arena, render_node);
    }
    
    for(u32 I=0; I<RenderGroupID_TOTAL; I++){
        RenderGroups[I].Node = ArenaPushType(Arena, render_node);
    }
    
    //~ Lights
    Lights = MakeArray<render_light>(Arena, MAX_LIGHT_COUNT);
    
    //~ Camera
    CameraTargetP.X = Clamp(CameraTargetP.X, CameraBounds.Min.X, CameraBounds.Max.X);
    CameraTargetP.Y = Clamp(CameraTargetP.Y, CameraBounds.Min.Y, CameraBounds.Max.Y);
    
    v2 Delta = dTime*CameraSpeed*(CameraTargetP-CameraFinalP);
    CameraFinalP += Delta;
    
    v2 BoundsSize = RectSize(CameraBounds);
    f32 Factor = 210.0f;
    f32 NewScale = Minimum(OutputSize.X/Factor, OutputSize.Y/Factor);
    NewScale = Maximum(NewScale, 1.0f);
    ChangeScale(NewScale);
}

//~ Render groups
void 
game_renderer::RenderGroupInitialize(u32 ID, basic_shader *Shader, f32 Scale){
    Assert(ID < RenderGroupID_TOTAL);
    RenderGroups[ID].Renderer = this;
    RenderGroups[ID].Shader   = Shader;
    RenderGroups[ID].Scale    = Scale;
}

render_group *
game_renderer::GetRenderGroup(u32 ID){
    Assert(ID < RenderGroupID_TOTAL);
    render_group *Result = &RenderGroups[ID];
    return Result;
}

render_item *
game_renderer::NewRenderItem(render_group *Group, render_texture Texture, b8 HasAlpha, z_layer Z){
    render_node *Node = Group->Node;
    if(Node->Count >= RENDER_NODE_ITEMS){
        render_node *OldNode = Node;
        Node = ArenaPushType(&GlobalTransientMemory, render_node);
        Node->Next = OldNode;
        Group->Node = Node;
    }
    
    u32 Index = Node->Count++;
    render_item *Result = &Node->Items[Index];
    Result->ClipRect = StackPeek(&ClipRects);
    Result->Texture = Texture;
    if(HasAlpha){
        Node->ItemZs[Index] = Z.Z;
    }else{
        Node->ItemZs[Index] = 0;
    }
    RenderItemCount++;
    
    return(Result);
}

basic_vertex *
game_renderer::AddVertices(render_item *Item, u32 VertexCount){
    Item->VertexOffset = Vertices.Count;
    Item->VertexCount = VertexCount;
    basic_vertex *Result = ArrayAlloc(&Vertices, VertexCount);
    return(Result);
}

u32 *
game_renderer::AddIndices(render_item *Item, u32 IndexCount){
    Item->IndexOffset  = Indices.Count;
    Item->IndexCount   = IndexCount;
    u32 *Result = ArrayAlloc(&Indices, IndexCount);
    return(Result);
}

v2 
game_renderer::CalculateParallax(s8 Layer){
    v2 Result = V2(0);
    
    if(Layer != 0){
        f32 Factor = 1.0f / (f32)Layer;
        Result = CameraFinalP * Factor;
    }
    Result = V2Round(Result);
    
    return(Result);
}

void
game_renderer::DoParallax(render_item *Item, s8 Layer){
    v2 Offset = CalculateParallax(Layer);
    
    for(u32 I=Item->VertexOffset; 
        I<(Item->VertexOffset+Item->VertexCount); 
        I++){
        basic_vertex *Vertex = &Vertices[I];
        
        Vertex->P -= Offset;
    }
    
}

void
game_renderer::BeginClipRect(rect ClipRect){
    StackPush(&ClipRects, RectRectify(ClipRect));
}

void
game_renderer::EndClipRect(){
    StackPop(&ClipRects);
}

//~ Light stuff
void
game_renderer::AddLight(v2 P, z_layer Z, color Color, f32 Intensity, f32 Radius){
    render_light *Light = ArrayAlloc(&Lights);
    *Light = {};
    
    Light->P = P - CalculateParallax(Z.Layer);
    Light->Z = (f32)Z.Z;
    Light->R = Intensity*Color.R;
    Light->G = Intensity*Color.G;
    Light->B = Intensity*Color.B;
    Light->Radius = Radius;
}

void 
game_renderer::SetLightingConditions(color AmbientLight_, f32 Exposure_){
    AmbientLight = AmbientLight_;
    Exposure     = Exposure_;
}

//~ Camera stuff

void
game_renderer::SetCameraSettings(f32 Speed){
    CameraSpeed = Speed;
}

void 
game_renderer::SetCameraTarget(v2 Center){
    v2 ScreenSize = OutputSize/CameraScale;
    CameraTargetP = Center - 0.5f*ScreenSize;
#if 0
    CameraFinalP = Center - 0.5f*ScreenSize;
    CameraFinalP.X = Clamp(CameraFinalP.X, CameraBounds.Min.X, CameraBounds.Max.X);
    CameraFinalP.Y = Clamp(CameraFinalP.Y, CameraBounds.Min.Y, CameraBounds.Max.Y);
#endif
}

void
game_renderer::MoveCamera(v2 Delta){
    CameraTargetP += Delta;
#if 0
    CameraFinalP += Delta;
    CameraFinalP.X = Clamp(CameraFinalP.X, CameraBounds.Min.X, CameraBounds.Max.X);
    CameraFinalP.Y = Clamp(CameraFinalP.Y, CameraBounds.Min.Y, CameraBounds.Max.Y);
#endif
}

void
game_renderer::ResetCamera(){
    CameraTargetP = {};
    CameraFinalP  = {};
}

void 
game_renderer::CalculateCameraBounds(world_data *World){
    v2 MapSize = TILE_SIDE*V2((f32)World->Width, (f32)World->Height);
    v2 ScreenSize = OutputSize/CameraScale;
    CameraBounds.Min = V2(0);
    CameraBounds.Max = MapSize-ScreenSize;
}


v2
game_renderer::WorldToScreen(v2 P, s8 Layer){
    v2 Offset = CalculateParallax(Layer);
    v2 Result = (P - Offset) * CameraScale;
    return(Result);
}

rect
game_renderer::WorldToScreen(rect R, s8 Layer){
    rect Result;
    Result.Min = WorldToScreen(R.Min, Layer);
    Result.Max = WorldToScreen(R.Max, Layer);
    return(Result);
}

v2
game_renderer::ScreenToWorld(v2 P, s8 Layer){
    v2 Offset = CalculateParallax(Layer);
    v2 Result = P / CameraScale + Offset;
    return(Result);
}

rect
game_renderer::ScreenToWorld(rect R, s8 Layer){
    rect Result;
    Result.Min = ScreenToWorld(R.Min, Layer);
    Result.Max = ScreenToWorld(R.Max, Layer);
    return(Result);
}\