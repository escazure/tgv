#version 330 core

in vec2 uv;
out float FragColor;

uniform sampler2D uHeightMap;
uniform vec3 uLightDir;
uniform float uTerrainSize;
uniform float uMaxHeight;

const int MAX_STEPS = 1024;
const float TEXELS_PER_STEP = 8.0;

void main(){
	float texelSize = 1.0 / uTerrainSize;
    vec3 fragToLight = normalize(-uLightDir);

	float horizontalLen = length(fragToLight.xz);
	if(horizontalLen < 0.0001){
		FragColor = 1.0;	
		return;
	}

	vec2 uvStep = (fragToLight.xz / horizontalLen) * texelSize * TEXELS_PER_STEP;
	float heightStep = (fragToLight.y / horizontalLen) * (texelSize * uTerrainSize) * TEXELS_PER_STEP;

	vec2 currentUV = uv + (uvStep * 2.0);
	float currentHeight = texture(uHeightMap, uv).r + (heightStep * 2.0);

	float shadow = 1.0;
	float softness = 4.0;
	float stepDistWorld = length(uvStep) * uTerrainSize;

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

		float distanceTraveled = float(i + 1) * stepDistWorld;
		shadow = min(shadow, softness * diff / distanceTraveled);

		currentUV += uvStep;
		currentHeight += heightStep;
    }

    FragColor = clamp(shadow, 0.0, 1.0);
}
