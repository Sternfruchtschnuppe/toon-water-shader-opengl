#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

uniform mat4 modelview;
uniform mat4 projection;
uniform mat3 normal;

uniform bool isWater;

uniform bool animateWater;

uniform float time;
layout(binding = 1) uniform sampler2D noiseSampler;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec3 pos_out;
out layout(location = 2) vec2 textureCoordinates_out;
out layout(location = 3) float wave_height_out;


//constants
float waveHeight = 0.25;
float waterScale = 0.1;
float waterSpeed = 0.003;

float waveFunction(vec2 coords){ return texture(noiseSampler, coords * waterScale + time * waterSpeed).x * waveHeight; }

void main()
{
    float offY = 0;
    vec3 norm = normal_in;

    if(isWater && animateWater){
        offY = waveFunction(position.xz);
        float dist = 0.01;
        float yl = waveFunction(position.xz + vec2(-dist, 0));
        float yr = waveFunction(position.xz + vec2(dist, 0));
        float yu = waveFunction(position.xz + vec2(0, dist));
        float yd = waveFunction(position.xz + vec2(0, -dist));
    
        vec3 v1 = vec3(2 * dist, yr - yl, 0);
        vec3 v2 = vec3(0, yu - yd, 2 * dist);

        norm = normalize(cross(v2, v1));
    }

    wave_height_out = offY / waveHeight;
    normal_out = normalize(normal * norm);
    pos_out = position + vec3(0,offY,0);
    gl_Position = modelview * vec4(pos_out, 1.0);
    pos_out = gl_Position.xyz;
    gl_Position = projection * gl_Position;
    textureCoordinates_out = textureCoordinates_in;
}