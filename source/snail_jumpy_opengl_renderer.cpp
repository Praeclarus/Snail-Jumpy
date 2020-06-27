#define X(Name) global type_##Name *Name;
OPENGL_FUNCTIONS
#undef X

global basic_program GlobalTextureShaderProgram;
global_constant char *TextureVertexShaderSource =
"#version 330 core \n"
"layout (location = 0) in vec3 Position;"
"layout (location = 1) in vec4 Color;"
"layout (location = 2) in vec2 TexCoord;"
"out vec2 FragmentTexCoord;"
"out vec4 FragmentColor;"
"uniform mat4 Projection;"
"void main(){"
"    gl_Position = Projection * vec4(Position, 1.0);"
"    FragmentTexCoord = TexCoord;"
"    FragmentColor = Color;"
"}";
global_constant char *TextureFragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;"
"in vec2 FragmentTexCoord;"
"in vec4 FragmentColor;"
"uniform sampler2D Texture;"
"void main()"
"{"
"    vec4 Color = texture(Texture, FragmentTexCoord) * FragmentColor;"
"    if(Color.a == 0.0){"
"        discard;"
"    }"
"    FragColor = Color;"
"}";

internal basic_program
GlCompileShaderProgram(const char *VertexShaderSource, const char *FragmentShaderSource){
    
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, 0);
    glCompileShader(VertexShader);
    {
        s32 Status;
        char Buffer[512];
        glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            // TODO(Tyler): Logging
            glGetShaderInfoLog(VertexShader, 512, 0, Buffer);
            LogError(Buffer);
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
            // TODO(Tyler): Logging
            glGetShaderInfoLog(FragmentShader, 1024, 0, Buffer);
            LogError(Buffer);
            Assert(0);
        }
    }
    basic_program Result;
    Result.Id = glCreateProgram();
    glAttachShader(Result.Id, VertexShader);
    glAttachShader(Result.Id, FragmentShader);
    glLinkProgram(Result.Id);
    {
        s32 Status;
        char Buffer[1024];
        glGetProgramiv(Result.Id, GL_LINK_STATUS, &Status);
        if(!Status){
            // TODO(Tyler): Logging
            glGetProgramInfoLog(Result.Id, 1024, 0, Buffer);
            LogError(Buffer);
            Assert(0);
        }
    }
    
    glUseProgram(Result.Id);
    Result.ProjectionLocation = glGetUniformLocation(Result.Id, "Projection");
    if(Result.ProjectionLocation == -1){
        LogError("Couldn't find the location of the 'Projection' uniform");
    }
    Assert(Result.ProjectionLocation != -1);
    return(Result);
}

//~ Render API

internal
INITIALIZE_RENDERER(InitializeRenderer){
    GlobalTextureShaderProgram =
        GlCompileShaderProgram(TextureVertexShaderSource, TextureFragmentShaderSource);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);
    
    GLuint VertexBuffer;
    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    
    GLuint ElementBuffer;
    glGenBuffers(1, &ElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, P));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, Color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertex), (void*)offsetof(vertex, TexCoord));
    glEnableVertexAttribArray(2);
    
    u8 TemplateColor[] = {0xff, 0xff, 0xff, 0xff};
    DefaultTexture = CreateRenderTexture(TemplateColor, 1, 1);
    
    b32 Result = true;
    return(Result);
}

// TODO(Tyler): Is there a command waiting for previous commands to finish and causing
// wasting a large number of clock cycles
internal
RENDER_GROUP_TO_SCREEN(RenderGroupToScreen){
    TIMED_FUNCTION();
    
    glViewport(0, 0,
               (GLsizei)RenderGroup->OutputSize.Width,
               (GLsizei)RenderGroup->OutputSize.Height);
    glClearColor(RenderGroup->BackgroundColor.R,
                 RenderGroup->BackgroundColor.G,
                 RenderGroup->BackgroundColor.B,
                 RenderGroup->BackgroundColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    f32 A = 2.0f/((f32)RenderGroup->OutputSize.Width);
    f32 B = 2.0f/((f32)RenderGroup->OutputSize.Height);
    f32 C = 2.0f/((f32)100);
    f32 Projection[] = {
        A,   0, 0, 0,
        0,   B, 0, 0,
        0,   0, C, 0,
        -1, -1, 0, 1,
    };
    
    glUseProgram(GlobalTextureShaderProgram.Id);
    glUniformMatrix4fv(GlobalTextureShaderProgram.ProjectionLocation, 1, GL_FALSE, Projection);
    
    glBufferData(GL_ARRAY_BUFFER,
                 RenderGroup->VertexCount*sizeof(vertex), RenderGroup->Vertices,
                 GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 RenderGroup->IndexCount*sizeof(u32), RenderGroup->Indices,
                 GL_STREAM_DRAW);
    
    u32 IndexOffset = 0;
    for(u32 Index = 0; Index < RenderGroup->OpaqueItems.Count; Index++){
        render_item *Item = &RenderGroup->OpaqueItems[Index];
        glBindTexture(GL_TEXTURE_2D, Item->Texture);
        glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)Item->IndexCount, GL_UNSIGNED_SHORT, (void*)(IndexOffset*sizeof(u16)), Item->VertexOffset);
        IndexOffset += Item->IndexCount;
    }
    
    f32 LastZ = 0.0f;
    for(u32 Index = 0; Index < RenderGroup->TranslucentItems.Count; Index++){
        render_item *Item = &RenderGroup->TranslucentItems[Index];
        
        if(LastZ != 0.0f) Assert((Item->ZLayer <= LastZ));
        LastZ = Item->ZLayer;
        
        glBindTexture(GL_TEXTURE_2D, Item->Texture);
        glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)Item->IndexCount, GL_UNSIGNED_SHORT, (void*)(IndexOffset*sizeof(u16)), Item->VertexOffset);
        IndexOffset += Item->IndexCount;
    }
}

internal
CREATE_RENDER_TEXTURE(CreateRenderTexture){
    u32 Result;
    glGenTextures(1, &Result);
    glBindTexture(GL_TEXTURE_2D, Result);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return(Result);
}

