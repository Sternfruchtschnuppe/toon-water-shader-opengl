#version 430 core

in layout(location = 0) vec3 normal;
in layout(location = 1) vec3 position;
in layout(location = 2) vec2 texCoords;
in layout(location = 3) float waveHeight;

uniform vec3 lightDir;
uniform float time;
uniform float width;
uniform float height;

uniform bool isWater;

uniform vec3 mainColor;
uniform vec3 secondaryColor;
uniform float textureID;

uniform bool notRenderDepth;
uniform bool distortion;
uniform bool notDistortAboveObjects;
uniform bool waterDepth;
uniform bool posterizeWater;
uniform bool foam;
uniform bool waterSpec;
uniform bool colorWaveHeight;
uniform bool smoothDiff;
uniform bool specular;
uniform bool rim;

layout(binding = 0) uniform sampler2D depthTexture;
layout(binding = 1) uniform sampler2D noiseSampler;
layout(binding = 2) uniform sampler2D flagSampler;
layout(binding = 3) uniform sampler2D shipBaseSampler;
layout(binding = 4) uniform sampler2D shipBase2Sampler;

out vec4 color;

//constants
float near = 0.1f;
float far = 50.0f;
float depthWorldUnitScale = 20.0f;
float noiseFoamCutoff = .55f;
int posterizationLevels = 4;
float timeScale = 0.01f;
float foamCutoffThreshold = 0.1f;
float waterSpecCutoff = 0.95f;
float distStr = 20.0f;
vec2 distScl = vec2(.005f, 0.002f);
vec2 randomOffset = vec2(.1231, .76242);

float linearizeDepth(float z) { return near * (z + 1.0) / (far + near - z * (far - near)); }
float sampleLinearDepth(vec2 coords) { return linearizeDepth(texture(depthTexture, coords / vec2(width, height)).x);}
float posterize(float val, int levels) { return floor(val * levels) / levels + 1.0 / levels; }
float saturate(float val) { return val < 0 ? 0 : val > 1 ? 1 : val; }

void main() {
  vec4 fragCoord = gl_FragCoord;
  vec3 N = normalize(normal);
  vec3 V = normalize(-position);
  vec3 R = reflect(-lightDir, N);

  float nDotL = dot(N, lightDir);
  float vDotR = dot(V, R);
  float nDotV = dot(V, N);

  if (!notRenderDepth)
  {
    color = vec4(vec3(pow(fragCoord.z, 32)), 1);
    if (isWater){
      color = vec4(0);
    }
    return;
  }

  if(isWater){
    float waterFragDepth = linearizeDepth(fragCoord.z);
    float fragDepthDelta = 0;
    if(distortion){
      //distortion
      float offX = texture(noiseSampler, texCoords * randomOffset.x - time * distScl.x).x;
      float offY = texture(noiseSampler, texCoords * randomOffset.y - time * distScl.x).x;
      float offX2 = texture(noiseSampler, texCoords * randomOffset.x + time * distScl.y).x;
      float offY2 = texture(noiseSampler, texCoords * randomOffset.y + time * distScl.y).x;

      vec2 dist = distStr * (vec2(offX + offX2, offY + offY2) * 2 - 2);
      
      float fragDepth = sampleLinearDepth(fragCoord.xy);
      float distFragDepth = sampleLinearDepth(fragCoord.xy + dist);
      fragDepthDelta = fragDepth - distFragDepth;
      if(!notDistortAboveObjects || fragDepth - distFragDepth <= 0.005){
        fragCoord.xy += dist;
      }
    }

    float depthDelta = sampleLinearDepth(fragCoord.xy) - waterFragDepth;
    depthDelta = saturate(depthDelta * depthWorldUnitScale);
    
    if (waterDepth){
      float colorFadeInfluence = posterizeWater ? posterize(depthDelta, posterizationLevels) : depthDelta;
      colorFadeInfluence = saturate(colorFadeInfluence);
      color = vec4(mainColor * colorFadeInfluence + secondaryColor * (1-colorFadeInfluence), 1);
    }
    else {
      color = vec4(0.5 * (mainColor + secondaryColor), 1);
    }
      
    if(foam){
      color += vec4(vec3(1 - smoothstep(foamCutoffThreshold - 0.005, foamCutoffThreshold + 0.005,  depthDelta)),0);
    }

    if(waterSpec){
      float spec = smoothstep(waterSpecCutoff - 0.005, waterSpecCutoff + 0.005, vDotR);
      color += vec4(vec3(spec),0);
    }

    if(colorWaveHeight){
      color = vec4(0.8 * color.xyz + 0.4 * vec3(waveHeight), 1);
    }

    return;
  }

  float ka = 0.4;
  float kd = 0.5;
  float ks = 0.2;

  float sumCoeff = ka;
  
  float diff;
  if(smoothDiff){
    diff = smoothstep(0, 0.01, nDotL);
  }else{
    diff = nDotL;
  }
  sumCoeff += kd * diff;

  if(specular){
    float spec = smoothstep(0.005, 0.007, max(pow(vDotR, 64), 0.0));
    sumCoeff += ks * spec;
  }
  
  if(rim){
    float kr = 0.4;
    float rimThickn = 0.7;
    float rimThresh = 0.1;
    sumCoeff += kr * smoothstep(
      rimThickn - 0.01, 
      rimThickn + 0.01, 
      (1-nDotV) * pow(nDotL, rimThresh));
  }

  vec4 texCol = vec4(1);
  if(textureID == 2){
    texCol = texture(flagSampler, texCoords);
  } else if (textureID == 3) {
    texCol = texture(shipBaseSampler, texCoords);
  } else if (textureID == 4) {
    texCol = texture(shipBaseSampler, texCoords);
  }
  
  //texCol = vec4(vec3(textureID * 0.25), 1);
  //texCol = texture(shipBase2Sampler, texCoords);
  color = texCol * vec4(mainColor * sumCoeff, 1);
}