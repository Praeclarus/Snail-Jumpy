#define X(Name) global type_##Name *Name;


OPENGL_FUNCTIONS
#undef X

internal GLuint
CompileShaderProgram(const char *VertexShaderSource, const char *FragmentShaderSource){
    
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
            Assert(0);
        }
    }
    
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, 0);
    glCompileShader(FragmentShader);
    {
        s32 Status;
        char Buffer[512];
        glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Status);
        if(!Status){
            // TODO(Tyler): Logging
            glGetShaderInfoLog(FragmentShader, 512, 0, Buffer);
            Assert(0);
        }
    }
    GLuint ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    {
        s32 Status;
        char Buffer[512];
        glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Status);
        if(!Status){
            // TODO(Tyler): Logging
            glGetProgramInfoLog(ShaderProgram, 512, 0, Buffer);
            Assert(0);
        }
    }
    return(ShaderProgram);
}

