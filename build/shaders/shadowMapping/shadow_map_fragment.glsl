#version 460 core
layout (binding = 0) uniform sampler2D uHeightMap;
layout (binding = 1) uniform sampler2D uNormalMap;

in vec2 uv;
out float FragColor;

uniform vec3 uLightDir;
uniform float uTerrainSize;
uniform float uMaxHeight;

const int MAX_STEPS = 1024;
const float BASE_TEXELS_PER_STEP = 2.0;
const float MAX_JUMP_FACTOR = 8.0;
const float SHADOW_SOFTNESS = 8.0;

float smin(float a, float b, float k){
    k *= 4.0;
    float h = max(k-abs(a-b), 0.0)/k;
    return min(a,b) - h*h*k*(1.0/4.0);
}

void main(){
	float texelSize = 1.0 / uTerrainSize;
    vec3 fragToLight = normalize(-uLightDir);

	float finalShadow = 1.0;

    if(fragToLight.y > 0.0){
        vec3 normal = textureLod(uNormalMap, uv, 0.0).rgb;

        if(dot(normal, fragToLight) > 0.0){
            float horizontalLen = length(fragToLight.xz);

            if(horizontalLen >= 0.0001){
                vec2 uvStep = (fragToLight.xz / horizontalLen) * texelSize * BASE_TEXELS_PER_STEP;
                float worldStepDist = length(uvStep) * uTerrainSize;
                float heightStep = (fragToLight.y / horizontalLen) * worldStepDist;

                vec4 currentRay = vec4(
                    uv + (normal.xz * 0.0001) + uvStep,
                    textureLod(uHeightMap, uv, 0.0).r + (heightStep * 2.0),
                    worldStepDist 
                );

                vec4 stepRay = vec4(uvStep, heightStep, worldStepDist);

                for(int i = 0; i < MAX_STEPS; i++){
                    if(any(lessThan(currentRay.xy, vec2(0.0))) || any(greaterThan(currentRay.xy, vec2(1.0)))) break;
                    if(currentRay.z > uMaxHeight) break;

                    float terrainHeight = textureLod(uHeightMap, currentRay.xy, 0.0).r;
                    float diff = currentRay.z - terrainHeight;

                    if(diff < 0.0){
                        finalShadow = 0.0;
                        break;
                    }

                    float stepMultiplier = clamp(1.0 + diff / (BASE_TEXELS_PER_STEP * 2.0), 1.0, MAX_JUMP_FACTOR);
                    finalShadow = smin(finalShadow, SHADOW_SOFTNESS * diff / currentRay.w, 0.1);

                    if(finalShadow < 0.01){
                        finalShadow = 0.0;
                        break;
                    }

                    currentRay += stepRay * stepMultiplier;
                }
            }
        }else{
            finalShadow = 0.0;
        }
    }else{
        finalShadow = 0.0;
    }

    FragColor = clamp(finalShadow, 0.0, 1.0);
}
