//~
#vertex_shader
#version 330 core

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
//~
#pixel_shader
#version 330 core

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

void main(){
    vec2 UV = FragmentUV;
    OutColor = texture(InTexture, UV)*FragmentColor;
    if(OutColor.a == 0.0){ discard; }
}