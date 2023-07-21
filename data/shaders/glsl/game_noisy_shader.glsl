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
    float Distance = distance(LightP.xy, FragmentP.xy);
    if(Distance > Radius) return vec3(0);
    float Attenuation = clamp(1.0 - (Distance*Distance)/(Radius*Radius), 0.0, 1.0);
    Attenuation *= Attenuation;
    vec3 Result = Attenuation*LightColor;
    return(Result);
}

float random(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// 2D Noise based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    
    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    
    // Smooth Interpolation
    
    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);
    
    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
    (c - a)* u.y * (1.0 - u.x) +
    (d - b) * u.x * u.y;
}

float Dampen(float A){
    A -= 0.5;
    return 1.0+0.5*A*abs(A);
}

void main(){
    vec2 UV = FragmentUV;
    OutColor = texture(InTexture, UV)*FragmentColor;
    
    vec3 AmbientLight = InAmbient.rgb;
    float Exposure = InExposure;
    
#if 0    
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
#endif
    vec2 NoiseP = 10*vec2(FragmentUV.x, FragmentUV.y);
    NoiseP.x -= 0.8*InTime;
    NoiseP.y += 0.5*InTime;
    OutColor *= Dampen(noise(NoiseP)); 
    
    if(OutColor.a == 0.0){ discard; }
}