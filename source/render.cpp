
//~ Basic rendering primitives

internal inline b8
ColorHasAlpha(color C){
    b8 Result = (C.A != 0.0f) && (C.A < 1.0f);
    return(Result);
}

internal void
RenderQuad(render_texture Texture, render_options Options, f32 Z,
           v2 P0, v2 T0, color C0, 
           v2 P1, v2 T1, color C1, 
           v2 P2, v2 T2, color C2, 
           v2 P3, v2 T3, color C3,
           b8 HasAlpha_=false){
    b8 HasAlpha = (ColorHasAlpha(C0) ||
                   ColorHasAlpha(C1) ||
                   ColorHasAlpha(C2) ||
                   ColorHasAlpha(C3)||
                   HasAlpha_);
    
    render_item *RenderItem = GameRenderer.NewRenderItem(Texture, Options, HasAlpha, Z);
    
    basic_vertex *Vertices = GameRenderer.AddVertices(RenderItem, 4);
    Vertices[0] = {P0, Z, T0, C0};
    Vertices[1] = {P1, Z, T1, C1};
    Vertices[2] = {P2, Z, T2, C2};
    Vertices[3] = {P3, Z, T3, C3};
    
    GameRenderer.DoParallax(RenderItem, Options, 4);
    
    u32 *Indices = GameRenderer.AddIndices(RenderItem, 6);
    Indices[0] = 0;
    Indices[1] = 1;
    Indices[2] = 2;
    Indices[3] = 0;
    Indices[4] = 2;
    Indices[5] = 3;
}

internal void
RenderLine(v2 A, v2 B, f32 Z, f32 Thickness, color Color, render_options Options){
    v2 AB = B - A;
    v2 AB90 = Normalize(Clockwise90(AB));
    v2 P0 = A - 0.5f*Thickness*AB90;
    v2 P1 = A + 0.5f*Thickness*AB90;
    v2 P3 = B - 0.5f*Thickness*AB90;
    v2 P2 = B + 0.5f*Thickness*AB90;
    
    RenderQuad(GameRenderer.WhiteTexture, Options, Z,
               P0, V2(0, 0), Color,
               P1, V2(0, 1), Color,
               P2, V2(1, 1), Color,
               P3, V2(1, 0), Color);
}

internal void
RenderLineFrom(v2 A, v2 Delta, f32 Z, f32 Thickness, color Color, render_options Options){
    RenderLine(A, A+Delta, Z, Thickness, Color, Options);
}

internal void
RenderCircle(v2 P, f32 Radius, f32 Z, color Color, render_options Options,
             u32 Sides=30){
    
    f32 T = 0.0f;
    f32 Step = 1.0f/(f32)Sides;
    
    render_item *RenderItem = GameRenderer.NewRenderItem(GameRenderer.WhiteTexture, Options, ColorHasAlpha(Color), Z);
    
    basic_vertex *Vertices = GameRenderer.AddVertices(RenderItem, Sides+2);
    Vertices[0] = {P.X, P.Y, Z, Color.R, Color.G, Color.B, Color.A, 0.0f, 0.0f};
    for(u32 I = 0; I <= Sides; I++){
        v2 Offset = V2(Cos(T*TAU), Sin(T*TAU));
        Vertices[I+1] = {P+Radius*Offset, Z, V2(0.0f, 0.0f), Color};
        T += Step;
    }
    
    u32 *Indices = GameRenderer.AddIndices(RenderItem, Sides*3);
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
RenderRect(rect R, f32 Z, color Color, render_options Options){
    RenderQuad(GameRenderer.WhiteTexture, Options, Z,
               V2(R.Min.X, R.Min.Y), V2(0, 0), Color,
               V2(R.Min.X, R.Max.Y), V2(0, 1), Color,
               V2(R.Max.X, R.Max.Y), V2(1, 1), Color,
               V2(R.Max.X, R.Min.Y), V2(1, 0), Color);
}

internal inline void
RenderRectOutline(rect R, f32 Z, color Color, render_options Options, f32 Thickness=1){
    RenderRect(Rect(R.Min,                          V2(R.Max.X, R.Min.Y+Thickness)), Z, Color, Options);
    RenderRect(Rect(V2(R.Max.X-Thickness, R.Min.Y), V2(R.Max.X, R.Max.Y)),           Z, Color, Options);
    RenderRect(Rect(V2(R.Min.X, R.Max.Y),           V2(R.Max.X, R.Max.Y-Thickness)), Z, Color, Options);
    RenderRect(Rect(V2(R.Min.X, R.Min.Y),           V2(R.Min.X+Thickness, R.Max.Y)), Z, Color, Options);
}

internal void
RenderTexture(rect R, f32 Z, render_texture Texture, render_options Options, 
              rect TextureRect=MakeRect(V2(0,0), V2(1,1)), b8 HasAlpha=false){
    Assert(Texture);
    
    color Color = WHITE;
    RenderQuad(Texture, Options, Z,
               V2(R.Min.X, R.Min.Y), V2(TextureRect.Min.X, TextureRect.Min.Y), Color,
               V2(R.Min.X, R.Max.Y), V2(TextureRect.Min.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Max.Y), V2(TextureRect.Max.X, TextureRect.Max.Y), Color,
               V2(R.Max.X, R.Min.Y), V2(TextureRect.Max.X, TextureRect.Min.Y), Color,
               HasAlpha);
}

//~ String rendering

internal void
RenderString(font *Font, color Color, v2 P, f32 Z, const char *String){
    
    v2 OutputSize = GameRenderer.OutputSize;
    
    P.Y = OutputSize.Y - P.Y;
    
    u32 Length = CStringLength(String);
    
    render_options Options = {};
    render_item *RenderItem = GameRenderer.NewRenderItem(Font->Texture, Options, true, Z);
    
    basic_vertex *Vertices = GameRenderer.AddVertices(RenderItem, 4*Length);
    u32 VertexOffset = 0;
    for(char C = *String; C; C = *(++String)){
        stbtt_aligned_quad Q;
        stbtt_GetBakedQuad(Font->CharData,
                           Font->TextureWidth, Font->TextureHeight,
                           C-32, &P.X, &P.Y, &Q, 1);
        Q.y0 = OutputSize.Y - Q.y0;
        Q.y1 = OutputSize.Y - Q.y1;
        Vertices[VertexOffset]   = {V2(Q.x0, Q.y0), Z, V2(Q.s0, Q.t0), Color};
        Vertices[VertexOffset+1] = {V2(Q.x0, Q.y1), Z, V2(Q.s0, Q.t1), Color};
        Vertices[VertexOffset+2] = {V2(Q.x1, Q.y1), Z, V2(Q.s1, Q.t1), Color};
        Vertices[VertexOffset+3] = {V2(Q.x1, Q.y0), Z, V2(Q.s1, Q.t0), Color};
        
        VertexOffset += 4;
    }
    
    u32 *Indices = GameRenderer.AddIndices(RenderItem, 6*Length);
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
VRenderFormatString(font *Font, color Color, 
                    v2 P, f32 Z, const char *Format, va_list VarArgs){
    char Buffer[1024];
    stbsp_vsnprintf(Buffer, 1024, Format, VarArgs);
    RenderString(Font, Color, P, Z, Buffer);
}

internal inline void
RenderFormatString(font *Font, color Color, v2 P, f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    VRenderFormatString(Font, Color, P, Z, Format, VarArgs);
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
RenderCenteredString(font *Font, color Color, v2 Center,
                     f32 Z, const char *Format, ...){
    va_list VarArgs;
    va_start(VarArgs, Format);
    f32 Advance = VGetFormatStringAdvance(Font, Format, VarArgs);
    Center.X -= Advance/2.0f;
    VRenderFormatString(Font, Color, Center, Z, Format, VarArgs);
    va_end(VarArgs);
}

//~ Render options
internal inline render_options
GameItem(u32 Layer){
    render_options Result = {};
    Result.Type = RenderType_Game;
    Result.Layer = Layer;
    return(Result);
}

internal inline render_options
ScaledItem(u32 Layer){
    render_options Result = {};
    Result.Type = RenderType_Scaled;
    Result.Layer = Layer;
    return(Result);
}

internal inline render_options
UIItem(u32 Layer){
    render_options Result = {};
    Result.Type = RenderType_UI;
    Result.Layer = Layer;
    return(Result);
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
    TIMED_FUNCTION();
    
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
void
game_renderer::Initialize(memory_arena *Arena, v2 OutputSize_){
    OutputSize = OutputSize_;
    
    //~ Camera
    CameraScale = 5;
    
    //~ Lights
    AmbientLight = 0.9f*Color(0.25f, 0.2f, 0.2f, 1.0f);
    Exposure = 1.2f;
    
    //~ Other
    u8 TemplateColor[] = {0xff, 0xff, 0xff, 0xff};
    GameRenderer.WhiteTexture = CreateRenderTexture(TemplateColor, 1, 1);
    
    GameShader       = MakeGameShader();
    GameScreenShader = MakeGameScreenShader();
    InitializeFramebuffer(&GameScreenFramebuffer, OutputSize/CameraScale);
    
    DefaultShader = MakeDefaultShader();
    
    DynamicArrayInitialize(&Vertices, 2000);
    DynamicArrayInitialize(&Indices,  2000);
}

void
game_renderer::ChangeScale(f32 NewScale){
    CameraScale = NewScale;
    ResizeFramebuffer(&GameScreenFramebuffer, OutputSize/CameraScale);
}

void
game_renderer::NewFrame(memory_arena *Arena, v2 OutputSize_, color ClearColor_){
    OutputSize = OutputSize_;
    ClearColor = ClearColor_;
    CurrentClipRect = SizeRect(V2(0), OutputSize);
    
    //~ Render items
    RenderItemCount = 0;
    DynamicArrayClear(&Vertices);
    DynamicArrayClear(&Indices);
    
    for(u32 I=0; I<RenderType_TOTAL; I++){
        Nodes[I] = PushStruct(Arena, render_node);
    }
    
    //~ Lights
    Lights = MakeNewArray<render_light>(Arena, RENDER_MAX_LIGHT_COUNT);
    
    //~ Camera
    CameraTargetP.X = Clamp(CameraTargetP.X, CameraBounds.Min.X, CameraBounds.Max.X);
    CameraTargetP.Y = Clamp(CameraTargetP.Y, CameraBounds.Min.Y, CameraBounds.Max.Y);
    
    // TODO(Tyler): This does allow for an the camera not to move when it might make sense
    // for it to. For instance when there is one pixel when the camera should be fully
    // to the right.
    v2 Delta = CameraSpeed*(CameraTargetP-CameraFinalP);
    Delta.X = Round(Delta.X);
    Delta.Y = Round(Delta.Y);
    CameraFinalP += Delta;
    
    v2 BoundsSize = RectSize(CameraBounds);
    f32 Factor = 215.0f;
    f32 NewScale = Minimum(OutputSize.X/Factor, OutputSize.Y/Factor);
    NewScale = Clamp(NewScale, 4.0f, 6.0f);
    ChangeScale(NewScale);
}

//~ Render stuff
render_item *
game_renderer::NewRenderItem(render_texture Texture, render_options Options, b8 HasAlpha, f32 Z){
    Assert(Options.Type < RenderType_TOTAL);
    
    render_node *Node = Nodes[Options.Type];
    if(Node->Count >= RENDER_NODE_ITEMS){
        render_node *OldNode = Node;
        Node = PushStruct(&TransientStorageArena, render_node);
        Node->Next = OldNode;
        Nodes[Options.Type] = Node;
    }
    
    u32 Index = Node->Count++;
    render_item *Result = &Node->Items[Index];
    Result->ClipRect = CurrentClipRect;
    Result->Texture = Texture;
    if(HasAlpha){
        Node->ItemZs[Index] = Z;
    }else{
        Node->ItemZs[Index] = F32_NEGATIVE_INFINITY;
    }
    RenderItemCount++;
    
    return(Result);
}

basic_vertex *
game_renderer::AddVertices(render_item *Item, u32 VertexCount){
    Item->VertexOffset = Vertices.Count;
    basic_vertex *Result = PushNArrayItems(&Vertices, VertexCount);
    return(Result);
}

u32 *
game_renderer::AddIndices(render_item *Item, u32 IndexCount){
    Item->IndexOffset  = Indices.Count;
    Item->IndexCount   = IndexCount;
    u32 *Result = PushNArrayItems(&Indices, IndexCount);
    return(Result);
}

v2 
game_renderer::CalculateParallax(render_options Options){
    v2 Result = V2(0);
    if(Options.Type == RenderType_UI) return(Result);
    
    if(Options.Layer > 0){
        f32 Factor = 1.0f / (f32)Options.Layer;
        Result = CameraFinalP * Factor;
    }
    
    if(Options.Type == RenderType_Game){
        Result.X = Round(Result.X);
        Result.Y = Round(Result.Y);
    }
    
    return(Result);
}

// TODO(Tyler): I don't like having to specify the vertex count for AddVertices and 
// this function but I don't currently see a better and simpler way.
void
game_renderer::DoParallax(render_item *Item, render_options Options, u32 VertexCount){
    v2 Offset = CalculateParallax(Options);
    
    for(u32 I=Item->VertexOffset; 
        I<(Item->VertexOffset+VertexCount); 
        I++){
        basic_vertex *Vertex = &Vertices[I];
        
        Vertex->P -= Offset;
    }
    
}

void
game_renderer::BeginClipRect(rect ClipRect){
    CurrentClipRect = ClipRect;
}

void
game_renderer::EndClipRect(){
    CurrentClipRect = SizeRect(V2(0), OutputSize);
}

//~ Light stuff
void
game_renderer::AddLight(v2 P, color Color, f32 Intensity, f32 Radius, render_options Options){
    render_light *Light = PushNewArrayItem(&Lights);
    *Light = {};
    
    Light->P = P - CalculateParallax(Options);
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
}

void
game_renderer::MoveCamera(v2 Delta){
    CameraTargetP += Delta;
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
game_renderer::WorldToScreen(v2 P, render_options Options){
    v2 Offset = CalculateParallax(Options);
    v2 Result = (P - Offset) * CameraScale;
    return(Result);
}

rect
game_renderer::WorldToScreen(rect R, render_options Options){
    rect Result;
    Result.Min = WorldToScreen(R.Min, Options);
    Result.Max = WorldToScreen(R.Max, Options);
    return(Result);
}

v2
game_renderer::ScreenToWorld(v2 P, render_options Options){
    v2 Offset = CalculateParallax(Options);
    v2 Result = P / CameraScale + Offset;
    return(Result);
}

rect
game_renderer::ScreenToWorld(rect R, render_options Options){
    rect Result;
    Result.Min = ScreenToWorld(R.Min, Options);
    Result.Max = ScreenToWorld(R.Max, Options);
    return(Result);
}