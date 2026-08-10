#version 330 core

in vec2 uv;
out float FragColor;

uniform sampler2D uHeightMap;
uniform vec3 uLightDir;
uniform float uTerrainSize;
uniform float uMaxHeight;

const int MAX_STEPS = 5000; // Higher = longer ray, less performance
const float STEP_SIZE = 0.1; // Less = more precise, shorter ray

void main(){
	float currentHeight = texture(uHeightMap, uv).r;
	vec3 worldPos = vec3(uv.x * uTerrainSize, currentHeight, uv.y * uTerrainSize);

    vec3 fragToLight = normalize(-uLightDir);
	worldPos += fragToLight * (STEP_SIZE * 2.0);

    for(int i = 0; i < MAX_STEPS; i++){
		vec2 sampleUV = worldPos.xz / uTerrainSize;

        if(sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0){
            break;
        }

		if(worldPos.y > uMaxHeight){
			break;
		}

        float terrainHeight = texture(uHeightMap, sampleUV).r;

        if(terrainHeight > worldPos.y){
            FragColor = 0.0;
            return;
        }

		worldPos += fragToLight * STEP_SIZE;
    }

    FragColor = 1.0;
}
