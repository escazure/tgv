#version 460 core
layout (location = 0) in vec2 aPos;
layout (binding = 0) uniform sampler2D uHeightMap;
layout (binding = 4) uniform sampler2D uErosionMask;

out vec3 WorldPos;
out vec2 uv;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform float uTerrainSize;
uniform bool uRenderTerrainSkirt;
uniform bool uApplyErosionMask;

void main(){
	uv = (aPos + vec2(uTerrainSize * 0.5)) / uTerrainSize;
	float height = texture(uHeightMap, uv).r;
	
	if(uApplyErosionMask)
		height -= texture(uErosionMask, uv).r;

	if(uRenderTerrainSkirt){
		bool isBorder = (uv.x <= 0.001 || uv.x >= 0.999 ||
    		             uv.y <= 0.001 || uv.y >= 0.999);

		if(isBorder) height = -500.0;
	}

	vec4 worldPos = uModel * vec4(aPos.x, height, aPos.y, 1.0);
	WorldPos = vec3(worldPos);
	gl_Position = uProjection * uView * worldPos;
}

