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
    int InZ = int(InPosition.z);
    InZ = ((InZ&0xf000) >> 12);
    FragmentP = vec3(InPosition.xy, float(InZ));
    FragmentColor = InColor;
    //FragmentColor = vec4(float(InZ)/6.0);
    FragmentUV = InPixelUV;
};
//~
#pixel_shader
#version 330 core

#define PI 3.1415926535897929

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
    float InTime;
    float InExposure;
    uint  InLightCount;
    light InLights[128];
};

vec3 CalculateLight(vec3 LightP, vec3 LightColor, float Radius){
    float ZFactor = 3.0;
    vec3 P = FragmentP;
    LightP.z *= ZFactor;
    P.z *= ZFactor;
    float Distance = distance(LightP, P);
    if(Distance > Radius) return vec3(0);
    float Attenuation = clamp(1.0 - (Distance*Distance)/(Radius*Radius), 0.0, 1.0);
    Attenuation *= Attenuation;
    vec3 Result = Attenuation*LightColor;
    return(Result);
}

void main(){
    vec2 UV = FragmentUV;
    OutColor = texture(InTexture, UV)*FragmentColor;
    
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
    
    if(OutColor.a == 0.0){ discard; }
}