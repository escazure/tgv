#version 460 core
layout (location = 0) out vec4 FragColor;

in vec3 worldPos;

uniform vec3 uLightDir;

const vec3 lightSkyBlue = vec3(210.0, 235.0, 255.0) / 255.0;
const vec3 deepSkyBlue = vec3(25.0, 105.0, 220.0) / 255.0;
const vec3 sunColor = vec3(1.0, 0.98, 0.9);

float getSunMask(float angularDist, float outerEdgeDeg, float innerEdgeDeg){
	float outerEdge = cos(radians(outerEdgeDeg));
	float innerEdge = cos(radians(innerEdgeDeg));
	return smoothstep(outerEdge, innerEdge, angularDist);
}

void main(){
	vec3 color = lightSkyBlue;
	vec3 fragToLight = normalize(-uLightDir);
	vec3 viewDir = normalize(worldPos);

	float angularDist = dot(viewDir, fragToLight);

	float heightFactor = viewDir.y;

	vec3 skyColor = mix(lightSkyBlue, deepSkyBlue, heightFactor);
	float sunGlow = pow(max(angularDist, 0.0), 8.0);
	vec3 skyWithGlow = skyColor + (skyColor * sunGlow * 0.3);

	float sunMask = getSunMask(angularDist, 1.0, 0.5);

	color = mix(skyWithGlow, sunColor, sunMask);

	FragColor = vec4(color, 1.0);
}
