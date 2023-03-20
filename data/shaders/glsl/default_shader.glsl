//~
#vertex_shader
#version 330 core

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

//~
#pixel_shader
#version 330 core

out vec4 OutColor;

in vec4 FragmentColor;
in vec2 FragmentUV;
uniform sampler2D InTexture;

void main(){
    OutColor = texture(InTexture, FragmentUV)*FragmentColor;
    if(OutColor.a == 0.0){ discard; }
}
