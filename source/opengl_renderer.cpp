#define X(Name) global type_##Name *Name;
OPENGL_FUNCTIONS
#undef X

//~ OpenGL backend
global opengl_backend OpenGL;

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
GLCompileShaderProgram(const char *VertexShaderSource, 
                       const char *FragmentShaderSource){
 
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

#if 0
// Stuff for 4Coder syntax highlighting in shaders

#define uniform
#define out
#define in
#define discard
#define gl_Position
struct vec2;
struct vec3;
struct vec4;
struct mat3;
struct mat4;
struct uint
struct sampler2D
struct light;
void CalculateLight();
void exp();
void clamp();
void distance();
void texture();
void textureSize();
void floor();
void fract();
void abs();
void InPosition;
void InColor;
void InUV;
void InPixelUV;

#endif

internal basic_shader
MakeGameShader(){
 const char *VertexShader = BEGIN_STRING
 (
#version 330 core \n
  
  layout (location = 0) in vec3 InPosition;
  layout (location = 1) in vec2 InPixelUV;
  layout (location = 2) in vec4 InColor;
  
  out vec3 FragmentP;
  out vec4 FragmentColor;
  out vec2 FragmentUV;
  uniform mat4 InProjection;
  
  void main(){
   gl_Position = InProjection * vec4(InPosition, 1.0);
   FragmentP = InPosition;
   FragmentColor = InColor;
   FragmentUV = InPixelUV;
  };
  );
 
 const char *FragmentShader = BEGIN_STRING
 (
#version 330 core \n
  
  struct light {
   vec3  P;
   float Radius;
   vec4  Color;
  };
  
  out vec4 OutColor;
  
  in vec3 FragmentP;
  in vec4 FragmentColor;
  in vec2 FragmentUV;
  
  uniform sampler2D InTexture;
  
  layout (std140) uniform LightsBlock{
   vec4 InAmbient;
   float InExposure;
   uint  InLightCount;
   light InLights[128];
  };
  
  vec3 CalculateLight(vec3 LightP, vec3 LightColor, float Radius){
   float ZScale = 0.5;
   LightP.z = ZScale*floor(LightP.z);
   vec3 P = vec3(FragmentP.xy, ZScale*floor(FragmentP.z));
   if(P.z >= 0.0){
    float Distance = distance(LightP, P);
    float Attenuation = clamp(1.0 - (Distance*Distance)/(Radius*Radius), 0.0, 1.0);
    Attenuation *= Attenuation;
    vec3 Result = Attenuation*LightColor;
    return(Result);
   }else{
    vec3 Result = vec3(0.0);
    return(Result);
   }
  }
  
  void main(){
   vec2 UV = FragmentUV;
   OutColor = texture(InTexture, UV)*FragmentColor;
   
   if(FragmentP.z >= 0.0){
    vec3 AmbientLight = InAmbient.rgb;
    float Exposure = InExposure;
    
    vec3 Lighting = vec3(0.0);
    
    for(int I = 0; I < int(InLightCount); I++){
     light Light = InLights[I];
     Lighting += CalculateLight(Light.P, Light.Color.rgb, Light.Radius);
    }
    OutColor *= vec4(AmbientLight+Lighting, 1.0);
    
    vec4 Vec4HDRColor = OutColor;
    vec3 HDRColor = Vec4HDRColor.rgb;
    vec3 MappedColor = vec3(1.0) - exp(-HDRColor*Exposure);
    OutColor = vec4(MappedColor, Vec4HDRColor.a);
   }
   
   if(OutColor.a == 0.0){ discard; }
  }
  
  );
 
 GLuint Program = GLCompileShaderProgram(VertexShader, FragmentShader);
 
 basic_shader Result = {};
 Result.ID = Program;
 
 glUseProgram(Program);
 Result.ProjectionLocation = glGetUniformLocation(Program, "InProjection");
 Assert(Result.ProjectionLocation != -1);
 u32 LightsBlockIndex = glGetUniformBlockIndex(Program, "LightsBlock");
 Assert(LightsBlockIndex != GL_INVALID_INDEX);
 glUniformBlockBinding(Program, LightsBlockIndex, 0);
 glBindBufferBase(GL_UNIFORM_BUFFER, 0, OpenGL.LightsUniformBuffer);
 
 return(Result);
}


internal basic_shader
MakeDefaultShader(){
 const char *VertexShader = BEGIN_STRING
 (
#version 330 core \n
  
  layout (location = 0) in vec3 InPosition;
  layout (location = 1) in vec2 InUV;
  layout (location = 2) in vec4 InColor;
  
  out vec4 FragmentColor;
  out vec2 FragmentUV;
  uniform mat4 InProjection;
  
  void main(){
   gl_Position = InProjection * vec4(InPosition, 1.0);
   FragmentColor = InColor;
   FragmentUV = InUV;
  };
  );
 
 const char *FragmentShader = BEGIN_STRING
 (
#version 330 core \n
  
  out vec4 OutColor;
  
  in vec4 FragmentColor;
  in vec2 FragmentUV;
  uniform sampler2D InTexture;
  
  void main(){
   OutColor = texture(InTexture, FragmentUV)*FragmentColor;
   if(OutColor.a == 0.0){ discard; }
  }
  
  );
 
 GLuint Program = GLCompileShaderProgram(VertexShader, FragmentShader);
 
 basic_shader Result = {};
 Result.ID = Program;
 
 glUseProgram(Program);
 Result.ProjectionLocation = glGetUniformLocation(Program, "InProjection");
 Assert(Result.ProjectionLocation != -1);
 
 return(Result);
};

internal basic_shader
MakeFontShader(){
 const char *VertexShader = BEGIN_STRING
 (
#version 330 core \n
  
  layout (location = 0) in vec3 InPosition;
  layout (location = 1) in vec2 InUV;
  layout (location = 2) in vec4 InColor;
  
  out vec4 FragmentColor;
  out vec2 FragmentUV;
  uniform mat4 InProjection;
  
  void main(){
   gl_Position = InProjection * vec4(InPosition, 1.0);
   FragmentColor = InColor;
   FragmentUV = InUV;
  };
  );
 
 const char *FragmentShader = BEGIN_STRING
 (
#version 330 core \n
  
  out vec4 OutColor;
  
  in vec4 FragmentColor;
  in vec2 FragmentUV;
  uniform sampler2D InTexture;
  
  void main(){
   OutColor = FragmentColor;
   OutColor *= vec4(1.0, 1.0, 1.0, texture(InTexture, FragmentUV).r);
   //OutColor = vec4(texture(InTexture, FragmentUV).rrrr);
   if(OutColor.a == 0.0){ discard; }
  }
  
  );
 
 GLuint Program = GLCompileShaderProgram(VertexShader, FragmentShader);
 
 basic_shader Result = {};
 Result.ID = Program;
 
 glUseProgram(Program);
 Result.ProjectionLocation = glGetUniformLocation(Program, "InProjection");
 Assert(Result.ProjectionLocation != -1);
 
 return(Result);
};


internal screen_shader
MakeGameScreenShader(){
 global_constant char *VertexShader = BEGIN_STRING
 (
#version 330 core \n
  
  layout (location = 0) in vec2 InPosition;
  layout (location = 1) in vec2 InUV;
  
  out vec2 FragmentUV;
  out vec2 FragmentP;
  
  void main(){
   gl_Position = vec4(InPosition, 0.0, 1.0);
   FragmentP = InPosition;
   FragmentUV = InUV;
  }
  );
 
 global_constant char *FragmentShader = BEGIN_STRING
 (
#version 330 core \n
  
  out vec4 OutColor;
  in vec2 FragmentUV;
  in vec2 FragmentP;
  
  uniform sampler2D InTexture;
  uniform float     InScale;
  
  void main(){
   //vec2 ScreenCenter = InOutputSize / 2.0;
   //vec2 Pixel = gl_FragCoord.xy;
   //vec2 TextureSize = textureSize(InTexture, 0).xy;
   
   //vec2 UV = FragmentUV;
   //vec2 UV = ScreenCenter;
   //float Scale = 4;
   //Pixel = (Pixel - ScreenCenter)/Scale + ScreenCenter;
   
   //vec2 UV = floor(Pixel)+0.5;
   //UV += 1.0 - clamp((1.0 - fract(Pixel)) * Scale, 0.0, 1.0);
   
   //UV /= TextureSize;
   //vec4 Color = vec4(Pixel/TextureSize, 0.0, 1.0);
   //vec2 UV = Pixel / InOutputSize;
   
   vec2 TextureSize = textureSize(InTexture, 0).xy;
   vec2 Pixel = FragmentUV*TextureSize;
   
   vec2 UV = floor(Pixel) + 0.5;
   
   //UV += 1.0 - clamp((1.0-fract(Pixel))*InScale, 0.0, 1.0);
   
   vec4 Color = texture(InTexture, UV/TextureSize);
   //vec4 Color = texture(InTexture, FragmentUV);
   if(Color.a == 0.0){
    discard;
   }
   
   OutColor = Color;
  }
  );
 
 GLuint Program = GLCompileShaderProgram(VertexShader, FragmentShader);
 
 screen_shader Result = {};
 Result.ID = Program;
 glUseProgram(Program);
 Result.ScaleLocation = glGetUniformLocation(Result.ID, "InScale");
 //Assert(Result.ScaleLocation != -1);
 
 return(Result);
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

void
opengl_backend::UploadLights(color AmbientColor, f32 Exposure, array<render_light> Lights){
 glBindBuffer(GL_UNIFORM_BUFFER, LightsUniformBuffer);
 
 u32 Size = (sizeof(opengl_lights_uniform_buffer) +
             Lights.Count*sizeof(opengl_light));
 opengl_lights_uniform_buffer *Buffer = (opengl_lights_uniform_buffer *)ArenaPush(&TransientStorageArena, Size);
 
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
opengl_backend::RenderFramebuffer(screen_shader *Shader, framebuffer *Framebuffer, f32 Scale){
 v2 OutputSize = OSInput.WindowSize;
 
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
GLRenderNodes(game_renderer *Renderer, render_node *StartNode,
              render_item_z *ZsA, render_item_z *ZsB, u32 ZItemCount){
 //~ Normal items
 u32 Index = 0; // Also doubles as the count for the current tree
 render_node *Node = StartNode;
 while(Node){
  for(u32 J=0; J<Node->Count; J++){
   render_item *Item = &Node->Items[J];
   
   if(Node->ItemZs[J] != F32_NEGATIVE_INFINITY){ 
    render_item_z *ZItem = &ZsA[Index++];
    ZItem->Z = Node->ItemZs[J];
    ZItem->Item = Item;
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
 
 //~ Alpha items, requiring sorting
 render_item_z *ZItems = MergeSortZs(ZsA, ZsB, Index);
 for(u32 I=0; I<Index; I++){
  render_item_z *ZItem = &ZItems[I];
  render_item *Item = ZItem->Item;
  if(!Item) continue;
  
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
 
 u32 ZItemCount = Renderer->RenderItemCount;
 render_item_z *ZsA = PushArray(&TransientStorageArena, render_item_z, ZItemCount);
 render_item_z *ZsB = PushArray(&TransientStorageArena, render_item_z, ZItemCount);
 
 //~ Pixel items
 GLUseBasicShader(&Renderer->GameShader, OutputSize, 1000);
 OpenGL.UploadLights(Renderer->AmbientLight, Renderer->Exposure, Renderer->Lights);
 
 UseFramebuffer(&Renderer->GameScreenFramebuffer);
 GLClearOutput(Renderer->ClearColor);
 GLRenderNodes(Renderer, Renderer->PixelNode, ZsA, ZsB, ZItemCount);
 OpenGL.RenderFramebuffer(&Renderer->GameScreenShader, &Renderer->GameScreenFramebuffer, Renderer->CameraScale);
 OpenGL.NormalSetup();
 
 //~ Scaled items
 GLUseBasicShader(&Renderer->DefaultShader, OutputSize, 1000, Renderer->CameraScale);
 GLRenderNodes(Renderer, Renderer->ScaledNode, ZsA, ZsB, ZItemCount);
 
 //~ Default items
 GLUseBasicShader(&Renderer->DefaultShader, OutputSize, 1000);
 GLRenderNodes(Renderer, Renderer->DefaultNode, ZsA, ZsB, ZItemCount);
 
 //~ Font item
 GLUseBasicShader(&Renderer->FontShader, OutputSize, 1000);
 GLRenderNodes(Renderer, Renderer->FontNode, ZsA, ZsB, ZItemCount);
}