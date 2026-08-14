#version 460 core
layout (binding = 0) uniform sampler2D uHeightMap;

in vec2 uv;
out vec4 FragColor;

uniform float uTerrainSize;

void main(){
	vec2 texelSize = 1.0 / textureSize(uHeightMap, 0);

	float worldStep = uTerrainSize * texelSize.x;

	float hL = texture(uHeightMap, uv + vec2(-texelSize.x, 0.0)).r;	
	float hR = texture(uHeightMap, uv + vec2(texelSize.x, 0.0)).r;	
	float hU = texture(uHeightMap, uv + vec2(0.0, texelSize.y)).r;	
	float hD = texture(uHeightMap, uv + vec2(0.0, -texelSize.y)).r;	

	vec3 dX = vec3(2.0 * worldStep, hR - hL, 0.0);
    vec3 dY = vec3(0.0, hU - hD, 2.0 * worldStep);

    vec3 normal = normalize(cross(dY, dX));
	
	FragColor = vec4(normal * 0.5 + 0.5, 1.0);
}
