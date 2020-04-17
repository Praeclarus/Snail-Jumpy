//~

internal
RENDER_GROUP_TO_SCREEN(Win32OpenGlRenderGroupToScreen){
    
    glViewport(0, 0,
               (GLsizei)RenderGroup->OutputSize.Width,
               (GLsizei)RenderGroup->OutputSize.Height);
    
    glClearColor(RenderGroup->BackgroundColor.R,
                 RenderGroup->BackgroundColor.G,
                 RenderGroup->BackgroundColor.B,
                 RenderGroup->BackgroundColor.A);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // TODO(Tyler): Clean this up
    local_persist GLuint TextureShaderProgram = 0;
    local_persist GLuint ColorShaderProgram = 0;
    if(TextureShaderProgram == 0){
        const char *TextureVertexShaderSource =
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
        const char *TextureFragmentShaderSource =
            "#version 330 core\n"
            "out vec4 FragColor;"
            "in vec2 FragmentTexCoord;"
            "in vec4 FragmentColor;"
            "uniform sampler2D Texture;"
            "void main()"
            "{"
            "    FragColor = texture(Texture, FragmentTexCoord) * FragmentColor;"
            "}";
        TextureShaderProgram =
            CompileShaderProgram(TextureVertexShaderSource, TextureFragmentShaderSource);
        
        const char *ColorVertexShaderSource =
            "#version 330 core \n"
            "layout (location = 0) in vec3 Position;"
            "layout (location = 1) in vec4 Color;"
            "out vec4 FragmentColor;"
            "uniform mat4 Projection;"
            "void main(){"
            "    gl_Position = Projection * vec4(Position, 1.0);"
            "    FragmentColor = Color;"
            "}";
        const char *ColorFragmentShaderSource =
            "#version 330 core\n"
            "out vec4 FragColor;"
            "in vec4 FragmentColor;"
            "uniform sampler2D Texture;"
            "void main()"
            "{"
            "    FragColor = FragmentColor;"
            "}";
        ColorShaderProgram = CompileShaderProgram(ColorVertexShaderSource, ColorFragmentShaderSource);
    }
    
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
    
    f32 A = 2.0f/((f32)RenderGroup->OutputSize.Width/RenderGroup->MetersToPixels);
    f32 B = 2.0f/((f32)RenderGroup->OutputSize.Height/RenderGroup->MetersToPixels);
    f32 Projection[] = {
        A,   0, 0, 0,
        0,   B, 0, 0,
        0,   0, 1, 0,
        -1, -1, 0, 1,
    };
    
    {
        glUseProgram(TextureShaderProgram);
        GLint ProjectionLocation = glGetUniformLocation(TextureShaderProgram, "Projection");
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, Projection);
    }{
        glUseProgram(ColorShaderProgram);
        GLint ProjectionLocation = glGetUniformLocation(ColorShaderProgram, "Projection");
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, Projection);
    }
    
    for(u32 Index = 0; Index < RenderGroup->Count; Index++){
        render_item *Item = &RenderGroup->Items[Index];
        GLuint ShaderProgram = ColorShaderProgram;
        if(Item->Texture){
            ShaderProgram = TextureShaderProgram;
            glBindTexture(GL_TEXTURE_2D, Item->Texture);
        }
        
        glUseProgram(ShaderProgram);
        
        glBufferData(GL_ARRAY_BUFFER, Item->VertexCount*sizeof(vertex), Item->Vertices, GL_STREAM_DRAW);
        
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, Item->IndexCount*sizeof(u32), Item->Indices, GL_STREAM_DRAW);
        
        glDrawElements(GL_TRIANGLES, (GLsizei)Item->IndexCount, GL_UNSIGNED_INT, 0);
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    glDeleteBuffers(1, &VertexBuffer);
    glDeleteBuffers(1, &ElementBuffer);
    glDeleteVertexArrays(1, &VertexArray);
}

internal
CREATE_RENDER_TEXTURE(Win32OpenGlCreateRenderTexture){
    u32 Result;
    glGenTextures(1, &Result);
    glBindTexture(GL_TEXTURE_2D, Result);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return(Result);
}

