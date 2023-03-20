#define GL_FUNC(Name) global type_##Name *Name;
OPENGL_FUNCTIONS
#undef GL_FUNC

//~ OpenGL backend
global opengl_backend OpenGL;
global_constant u32 BUFFER_LIGHTS_BINDING_POINT = 1;

internal b8
InitializeRendererBackend(){
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT,  1);
    
    //~ Setup default objects
    {
        glGenVertexArrays(1, &OpenGL.VertexArray);
        glBindVertexArray(OpenGL.VertexArray);
        
        glGenBuffers(1, &OpenGL.VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBuffer);
        
        GLuint ElementBuffer;
        glGenBuffers(1, &ElementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
        
        glGenBuffers(1, &OpenGL.LightsUniformBuffer);
        glBindBuffer(GL_UNIFORM_BUFFER, OpenGL.LightsUniformBuffer);
        u32 LightsBufferSize = (sizeof(opengl_lights_uniform_buffer) + 
                                MAX_LIGHT_COUNT*sizeof(opengl_light));
        glBufferData(GL_UNIFORM_BUFFER, LightsBufferSize, 0, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, BUFFER_LIGHTS_BINDING_POINT, OpenGL.LightsUniformBuffer);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(basic_vertex), (void*)offsetof(basic_vertex, P));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(basic_vertex), (void*)offsetof(basic_vertex, PixelUV));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(basic_vertex), (void*)offsetof(basic_vertex, Color));
        glEnableVertexAttribArray(2);
    }
    
    //~ Setup screen objects
    {
        glGenVertexArrays(1, &OpenGL.ScreenVertexArray);
        glBindVertexArray(OpenGL.ScreenVertexArray);
        
        local_constant float Vertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f,   1.0f,  1.0f, 1.0f, 
            1.0f,  -1.0f,  1.0f, 0.0f,
        };
        
        GLuint VertexBuffer;
        glGenBuffers(1, &VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    b8 Result = true;
    return(Result);
}

internal void
GLClearOutput(color Color){
    glClearColor(Color.R, Color.G, Color.B, Color.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
opengl_backend::NormalSetup(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    
    glBindVertexArray(VertexArray);
}

void
opengl_backend::UploadRenderData(dynamic_array<basic_vertex> *Vertices, 
                                 dynamic_array<u32> *Indices){
    glBindVertexArray(VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, Vertices->Count*sizeof(basic_vertex), Vertices->Items,
                 GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices->Count*sizeof(u32), Indices->Items,
                 GL_STREAM_DRAW);
}

internal GLuint
GLCompileShaderProgram(const char *VertexShaderSource, u32 VertexShaderLength,
                       const char *FragmentShaderSource, u32 FragmentShaderLength){
    
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, 0);
    glCompileShader(VertexShader);
    {
        s32 Status;
        char Buffer[512];
        glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(VertexShader, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, 0);
    glCompileShader(FragmentShader);
    {
        s32 Status;
        char Buffer[1024];
        glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(FragmentShader, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint Result = glCreateProgram();
    glAttachShader(Result, VertexShader);
    glAttachShader(Result, FragmentShader);
    glLinkProgram(Result);
    {
        s32 Status;
        char Buffer[1024];
        glGetProgramiv(Result, GL_LINK_STATUS, &Status);
        if(!Status){
            glGetProgramInfoLog(Result, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    return(Result);
}

//~ Shader programs
internal GLuint
GLCompileShaderProgram(const char *VertexShaderSource, const char *FragmentShaderSource){
    
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, 0);
    glCompileShader(VertexShader);
    {
        s32 Status;
        char Buffer[512];
        glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(VertexShader, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, 0);
    glCompileShader(FragmentShader);
    {
        s32 Status;
        char Buffer[1024];
        glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(FragmentShader, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint Result = glCreateProgram();
    glAttachShader(Result, VertexShader);
    glAttachShader(Result, FragmentShader);
    glLinkProgram(Result);
    {
        s32 Status;
        char Buffer[1024];
        glGetProgramiv(Result, GL_LINK_STATUS, &Status);
        if(!Status){
            glGetProgramInfoLog(Result, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    return(Result);
}

internal GLuint
GLCompileShaderProgram(const char *VertexShaderSource,   s32 VertexShaderLength,
                       const char *FragmentShaderSource, s32 FragmentShaderLength){
    
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, &VertexShaderLength);
    glCompileShader(VertexShader);
    {
        s32 Status;
        char Buffer[512];
        glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(VertexShader, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, &FragmentShaderLength);
    glCompileShader(FragmentShader);
    {
        s32 Status;
        char Buffer[1024];
        glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            glGetShaderInfoLog(FragmentShader, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    GLuint Result = glCreateProgram();
    glAttachShader(Result, VertexShader);
    glAttachShader(Result, FragmentShader);
    glLinkProgram(Result);
    {
        s32 Status;
        char Buffer[1024];
        glGetProgramiv(Result, GL_LINK_STATUS, &Status);
        if(!Status){
            glGetProgramInfoLog(Result, 1024, 0, Buffer);
            LogMessage(Buffer);
            Assert(0);
        }
    }
    
    return(Result);
}

//~ Shader programs from file

enum shader_region {
    ShaderRegion_None,
    ShaderRegion_Vertex,
    ShaderRegion_Pixel,
};

internal char *
ConsumeWhiteSpace(char *Result){
    char *NextBytePtr = Result;
    if(NextBytePtr){
        char NextByte = *NextBytePtr;
        if((NextByte == ' ') ||
           (NextByte == '\t') ||
           (NextByte == '\n') ||
           (NextByte == '\r')){
            b8 DoDecrement = true;
            while((NextByte == ' ') ||
                  (NextByte == '\t') ||
                  (NextByte == '\n') ||
                  (NextByte == '\r')){
                char *NextBytePtr = Result++;
                if(!NextBytePtr) { DoDecrement = false; break; }
                NextByte = *NextBytePtr;
            }
            if(DoDecrement) Result--;
        }
    }
    
    return Result;
}

// TODO(Tyler): I'm not sure how this should be structured, when we begin to
// use multiple rendering APIs
internal shader_program
MakeShaderProgramFromFileData(entire_file File){
    char *VertexSource = 0;
    s32 VertexLength = 0;
    char *FragmentSource = 0;
    s32 FragmentLength = 0;
    
    shader_region CurrentRegion = ShaderRegion_None;
    
    char *Start = (char *)File.Data; 
    char *Pointer = Start;
    while(*Pointer){
        if(*Pointer == '#'){
            if(IsStringASubset("#vertex_shader", Pointer)){
                u32 L = CStringLength("#vertex_shader");
                Pointer += L;
                Pointer = ConsumeWhiteSpace(Pointer);
                VertexSource = Pointer;
                CurrentRegion = ShaderRegion_Vertex;
                Assert(VertexLength == 0);
            }else if(IsStringASubset("#pixel_shader", Pointer)){
                u32 L = CStringLength("#pixel_shader");
                Pointer += L;
                Pointer = ConsumeWhiteSpace(Pointer);
                FragmentSource = Pointer;
                CurrentRegion = ShaderRegion_Pixel;
                Assert(FragmentLength == 0);
            }
        }
        
        if(CurrentRegion == ShaderRegion_Vertex){
            VertexLength++;
        }else if(CurrentRegion == ShaderRegion_Pixel){
            FragmentLength++;
        }
        
        Pointer++;
    }
    
    GLuint Result = GLCompileShaderProgram(VertexSource, VertexLength, FragmentSource, FragmentLength);
    
    return Result;
}

internal screen_shader
MakeScreenShaderFromFileData(entire_file File){
    GLuint Program = MakeShaderProgramFromFileData(File);
    
    screen_shader Result = {};
    Result.ID = Program;
    glUseProgram(Program);
    Result.ScaleLocation = glGetUniformLocation(Result.ID, "InScale");
    //Assert(Result.ScaleLocation != -1);
    
    return Result;
}

internal s32
ShaderProgramGetUniformLocation(shader_program Program, const char *Name){
    glUseProgram(Program);
    s32 Result = glGetUniformLocation(Program, Name);
    return Result;
}

//~ Shaders
internal void
GLUseBasicShader(basic_shader *Shader, v2 OutputSize, f32 ZResolution, f32 Scale=1){
    glUseProgram(Shader->ID);
    f32 A = 2.0f/(OutputSize.Width/Scale);
    f32 B = 2.0f/(OutputSize.Height/Scale);
    f32 C = 2.0f/(ZResolution);
    f32 Projection[] = {
        A,   0, 0, 0,
        0,   B, 0, 0,
        0,   0, C, 0,
        -1, -1, 0, 1,
    };
    glUniformMatrix4fv(Shader->ProjectionLocation, 1, GL_FALSE, Projection);
}

internal void
ShaderProgramSetupLighting(shader_program Program){
    glUseProgram(Program);
    GLuint Index = glGetUniformBlockIndex(Program, "LightsBlock");
    glUniformBlockBinding(Program, Index, BUFFER_LIGHTS_BINDING_POINT);
}

void
opengl_backend::UploadLights(color AmbientColor, f32 Exposure, array<render_light> Lights){
    glBindBuffer(GL_UNIFORM_BUFFER, LightsUniformBuffer);
    
    u32 Size = (sizeof(opengl_lights_uniform_buffer) +
                Lights.Count*sizeof(opengl_light));
    opengl_lights_uniform_buffer *Buffer = (opengl_lights_uniform_buffer *)ArenaPush(&GlobalTransientMemory, Size);
    
    Buffer->Time = Counter;
    Buffer->AmbientColor.R = AmbientColor.R;
    Buffer->AmbientColor.G = AmbientColor.G;
    Buffer->AmbientColor.B = AmbientColor.B;
    Buffer->Exposure = Exposure;
    Buffer->LightCount = Lights.Count;
    
    for(u32 I=0; I<Lights.Count; I++){
        render_light *Light   = &Lights[I];
        opengl_light *GLLight = &Buffer->Lights[I];
        GLLight->P = Light->P;
        GLLight->Z = Light->Z;
        GLLight->Radius = Light->Radius;
        GLLight->Color.R = Light->R;
        GLLight->Color.G = Light->G;
        GLLight->Color.B = Light->B;
    }
    
    glBufferSubData(GL_UNIFORM_BUFFER, 0, Size, Buffer);
}

//~ Framebuffer

internal void 
InitializeFramebuffer(framebuffer *Framebuffer, v2 Size){
    GLsizei Width = (GLsizei)Size.X;
    GLsizei Height = (GLsizei)Size.Y;
    
    glGenFramebuffers(1, &Framebuffer->ID);
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->ID);
    
    glGenTextures(1, &Framebuffer->Texture);
    glBindTexture(GL_TEXTURE_2D, Framebuffer->Texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Framebuffer->Texture, 0);
    
    glGenRenderbuffers(1, &Framebuffer->RenderbufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer->RenderbufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 
                              Framebuffer->RenderbufferID);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        LogMessage("ERROR: framebuffer not complete!");
        INVALID_CODE_PATH;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

internal void 
ResizeFramebuffer(framebuffer *Framebuffer, v2 NewSize){
    GLsizei Width = (GLsizei)NewSize.X;
    GLsizei Height = (GLsizei)NewSize.Y;
    
    glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->ID);
    
    glBindTexture(GL_TEXTURE_2D, Framebuffer->Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Framebuffer->Texture, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, Framebuffer->RenderbufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Width, Height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 
                              Framebuffer->RenderbufferID);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        LogMessage("ERROR: framebuffer not complete!");
        INVALID_CODE_PATH;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

internal void
UseFramebuffer(framebuffer *Framebuffer){
    if(Framebuffer){
        glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer->ID);
    }else{
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void
opengl_backend::RenderFramebuffer(screen_shader *Shader, framebuffer *Framebuffer, v2 OutputSize, f32 Scale){
    glScissor(0, 0, (u32)OutputSize.X, (u32)OutputSize.Y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    //glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(Shader->ID);
    glUniform1f(Shader->ScaleLocation, Scale);
    glBindVertexArray(ScreenVertexArray);
    glBindTexture(GL_TEXTURE_2D, Framebuffer->Texture);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

//~ Textures

internal render_texture
MakeTexture(texture_flags Flags){
    render_texture Result;
    glGenTextures(1, &Result);
    glBindTexture(GL_TEXTURE_2D, Result);
    
    if(Flags & TextureFlag_Blend){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }else{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    return(Result);
}

internal void
DeleteTexture(render_texture Texture){
    glDeleteTextures(1, &Texture);
}

internal void
TextureUpload(render_texture Texture, u8 *Pixels, u32 Width, u32 Height, u32 Channels){
    glBindTexture(GL_TEXTURE_2D, Texture);
    if(Channels == 1){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Width, Height, 0, GL_RED, GL_UNSIGNED_BYTE, Pixels);
    }else if(Channels == 4){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
    }else{ INVALID_CODE_PATH; }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

//~ Rendering

// TODO(Tyler): This might be able to be modified to merge sort each node 
// and then merge sort the nodes together, but right now this works just
// fine. I am also unsure of whether all node trees should be sorted together
// and then rendered in order, such would only work for RenderType_UI and RenderType_Scaled.
// I think it is fine that each tree is rendered separately.
internal inline void
GLRenderNodes(game_renderer *Renderer, shader_program ShaderID, render_node *StartNode,
              render_item_z *ZsA, u32 *ZCountOut){
    glUseProgram(ShaderID);
    //~ Normal items
    render_node *Node = StartNode;
    while(Node){
        for(u32 J=0; J<Node->Count; J++){
            render_item *Item = &Node->Items[J];
            
            if(Node->ItemZs[J] != 0){ 
                render_item_z *ZItem = &ZsA[(*ZCountOut)++];
                ZItem->Z = Node->ItemZs[J];
                ZItem->Item = Item;
                ZItem->ShaderID = ShaderID;
                continue; 
            }
            
            glBindTexture(GL_TEXTURE_2D, Item->Texture);
            v2 ClipSize = RectSize(Item->ClipRect);
            glScissor((GLsizei)Item->ClipRect.Min.X, (GLsizei)Item->ClipRect.Min.Y, 
                      (GLsizei)ClipSize.X,           (GLsizei)ClipSize.Y);
            glDrawElementsBaseVertex(GL_TRIANGLES, Item->IndexCount, GL_UNSIGNED_INT, 
                                     (void *)(Item->IndexOffset*sizeof(u32)),
                                     Item->VertexOffset);
        }
        
        Node = Node->Next;
    }
}
internal inline void
GLRenderAlphaNodes(game_renderer *Renderer, render_item_z *ZsA, render_item_z *ZsB, u32 Count){
    //~ Alpha items, requiring sorting
    render_item_z *ZItems = MergeSortZs(ZsA, ZsB, Count);
    for(u32 I=0; I<Count; I++){
        render_item_z *ZItem = &ZItems[I];
        render_item *Item = ZItem->Item;
        if(!Item) continue;
        
        glUseProgram(ZItem->ShaderID);
        glBindTexture(GL_TEXTURE_2D, Item->Texture);
        v2 ClipSize = RectSize(Item->ClipRect);
        glScissor((GLsizei)Item->ClipRect.Min.X, (GLsizei)Item->ClipRect.Min.Y, 
                  (GLsizei)ClipSize.X,           (GLsizei)ClipSize.Y);
        glDrawElementsBaseVertex(GL_TRIANGLES, Item->IndexCount, GL_UNSIGNED_INT, 
                                 (void *)(Item->IndexOffset*sizeof(u32)),
                                 Item->VertexOffset);
    }
}

internal void
RendererRenderAll(game_renderer *Renderer){
    TIMED_FUNCTION();
    
    v2 OutputSize = Renderer->OutputSize;
    glScissor(0, 0, (u32)OutputSize.X, (u32)OutputSize.Y);
    glViewport(0, 0, (u32)OutputSize.X, (u32)OutputSize.Y);
    
    OpenGL.UploadRenderData(&Renderer->Vertices, &Renderer->Indices);
    GLClearOutput(Renderer->ClearColor);
    OpenGL.NormalSetup();
    
    render_item_z *ZsA = ArenaPushArray(&GlobalTransientMemory, render_item_z, Renderer->RenderItemCount);
    render_item_z *ZsB = ArenaPushArray(&GlobalTransientMemory, render_item_z, Renderer->RenderItemCount);
    
    FOR_RANGE(I, 0, RenderGroupID_TOTAL){
        render_group *Group = &Renderer->RenderGroups[I];
        GLUseBasicShader(Group->Shader, OutputSize, U16_MAX, Group->Scale);
    }
    
    
    //~ Game items
    {
        glUseProgram(Renderer->RenderGroups[RenderGroupID_Lighting].Shader->ID);
        OpenGL.UploadLights(Renderer->AmbientLight, Renderer->Exposure, Renderer->Lights);
        UseFramebuffer(&Renderer->GameScreenFramebuffer);
        GLClearOutput(Renderer->ClearColor);
        render_group_id GameGroups[] = {RenderGroupID_Lighting, RenderGroupID_NoLighting, RenderGroupID_Noisy};
        u32 ZItemCount = 0;
        FOR_RANGE(I, 0, ArrayCount(GameGroups)){
            render_group *Group = &Renderer->RenderGroups[GameGroups[I]];
            
            GLRenderNodes(Renderer, Group->Shader->ID, Group->Node, ZsA, &ZItemCount);
        }
        GLRenderAlphaNodes(Renderer, ZsA, ZsB, ZItemCount);
        OpenGL.RenderFramebuffer(&Renderer->GameScreenShader, &Renderer->GameScreenFramebuffer, OutputSize, Renderer->CameraScale);
        OpenGL.NormalSetup();
    }
    
    //~ Default item
    {
        render_group_id NonGameGroups[] = {RenderGroupID_UI, RenderGroupID_Scaled, RenderGroupID_Font};
        u32 ZItemCount = 0;
        FOR_RANGE(I, 0, ArrayCount(NonGameGroups)){
            render_group *Group = &Renderer->RenderGroups[NonGameGroups[I]];
            GLRenderNodes(Renderer, Group->Shader->ID, Group->Node, ZsA, &ZItemCount);
        }
        
        GLRenderAlphaNodes(Renderer, ZsA, ZsB, ZItemCount);
    }
}