#version 330 core
layout (location = 0) in vec3 aPos;

uniform bool renderTerrainSkirt;
out vec3 WorldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float halfTerrainLength;

uniform sampler2D heightMap;

void main(){
	vec2 uv = (aPos.xz + vec2(halfTerrainLength)) / (2.0 * halfTerrainLength);
	float height = texture(heightMap, uv).r;

	if(renderTerrainSkirt){
		bool isBorder = (uv.x <= 0.001 || uv.x >= 0.999 ||
    		             uv.y <= 0.001 || uv.y >= 0.999);

		if(isBorder) height = -500.0;
	}

	vec4 worldPos = model * vec4(aPos.x, height, aPos.z, 1.0);
	WorldPos = vec3(worldPos);
	gl_Position = projection * view * worldPos;
}

