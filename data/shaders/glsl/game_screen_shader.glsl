#vertex_shader

#version 330 core

layout (location = 0) in vec2 InPosition;
layout (location = 1) in vec2 InUV;

out vec2 FragmentUV;
out vec2 FragmentP;

uniform float InScale;

void main(){
    gl_Position = vec4(InPosition, 0.0, 1.0);
    FragmentP = InPosition;
    FragmentUV = InUV;
}

//~
#pixel_shader

#version 330 core

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
    // vec2 UV = Pixel;
    
    //vec2 UV = FragmentUV;
    
    //UV += 1.0 - clamp((1.0-fract(Pixel))*InScale, 0.0, 1.0);
    
    vec4 Color = texture(InTexture, UV/TextureSize);
    //vec4 Color = texture(InTexture, FragmentUV);
    if(Color.a == 0.0){
        discard;
    }
    
    OutColor = Color;
}