#version 330 core

in vec2 uv;
out float FragColor;

uniform sampler2D uHeightMap;
uniform vec3 uLightDir;
uniform float uTerrainSize;
uniform float uMaxHeight;

const int MAX_STEPS = 1024;

void main(){
	float texelSize = 1.0 / uTerrainSize;
    vec3 fragToLight = normalize(-uLightDir);

	float horizontalLen = length(fragToLight.xz);
	if(horizontalLen < 0.0001){
		FragColor = 1.0;	
		return;
	}

	vec2 uvStep = (fragToLight.xz / horizontalLen) * texelSize;
	float heightStep = (fragToLight.y / horizontalLen) * (texelSize * uTerrainSize);

	vec2 currentUV = uv + (uvStep * 2.0);
	float currentHeight = texture(uHeightMap, uv).r + (heightStep * 2.0);

    for(int i = 0; i < MAX_STEPS; i++){
        if(currentUV.x < 0.0 || currentUV.x > 1.0 || currentUV.y < 0.0 || currentUV.y > 1.0)
			break;

		if(currentHeight > uMaxHeight)
			break;

        float terrainHeight = texture(uHeightMap, currentUV).r;

        if(terrainHeight > currentHeight){
            FragColor = 0.0;
            return;
        }

		currentUV += uvStep;
		currentHeight += heightStep;
    }

    FragColor = 1.0;
}
