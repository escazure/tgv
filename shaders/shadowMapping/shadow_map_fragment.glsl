#version 330 core

in vec2 uv;
out float FragColor;

uniform sampler2D uHeightMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMinMaxMap;
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

	if(fragToLight.y <= 0.0){
		FragColor = 0.0;
		return;
	}

	vec3 normal = texture(uNormalMap, uv).rgb;
	if(dot(normal, fragToLight) <= 0.0){
		FragColor = 0.0;
		return;
	}

	float horizontalLen = length(fragToLight.xz);
	if(horizontalLen < 0.0001){
		FragColor = 1.0;	
		return;
	}

	vec2 uvStep = (fragToLight.xz / horizontalLen) * texelSize * BASE_TEXELS_PER_STEP;

	float worldStepDist = length(uvStep) * uTerrainSize;
	float heightStep = (fragToLight.y / horizontalLen) * worldStepDist;

	vec2 currentUV = uv + (normal.xz * 0.0001) + (uvStep * 1.0);
	float currentHeight = texture(uHeightMap, uv).r + (heightStep * 2.0);

	float shadow = 1.0;

	float totalDistWorld = 0.0f;

    for(int i = 0; i < MAX_STEPS; i++){
        if(currentUV.x < 0.0 || currentUV.x > 1.0 || currentUV.y < 0.0 || currentUV.y > 1.0)
			break;

		if(currentHeight > uMaxHeight)
			break;

        float terrainHeight = texture(uHeightMap, currentUV).r;
		float diff = currentHeight - terrainHeight;

		if(diff < 0.0){
			FragColor = 0.0;
			return;
		}

		float stepMultiplier = clamp(1.0 + diff / (BASE_TEXELS_PER_STEP * 2.0), 1.0, MAX_JUMP_FACTOR);
		shadow = smin(shadow, SHADOW_SOFTNESS * diff / totalDistWorld, 0.1);

		if(shadow < 0.01)
			break;

		currentUV += uvStep * stepMultiplier;
		currentHeight += heightStep * stepMultiplier;
		totalDistWorld += worldStepDist * stepMultiplier;
    }

    FragColor = clamp(shadow, 0.0, 1.0);
}
