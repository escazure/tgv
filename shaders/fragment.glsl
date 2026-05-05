#version 330 core

in vec3 Normal;
in float normalized_y;

out vec4 FragColor;

uniform bool show_normals;

const vec3 up = vec3(0.0, 1.0, 0.0);

// Colors
const vec3 grass = vec3(0.2, 0.8, 0.2);
const vec3 rock = vec3(0.5, 0.5, 0.5);
const vec3 sand = vec3(0.7, 0.7, 0.5);
const vec3 snow = vec3(1.0, 1.0, 1.0);

void main(){
	vec3 normal = normalize(Normal);
	float slope = dot(normal, up);
	slope = clamp(slope, 0.0, 1.0);

	vec3 ground = mix(rock, grass, slope);
	ground = mix(sand, ground, smoothstep(0.0, 0.3, normalized_y));
	ground = mix(ground, rock, smoothstep(0.75, 0.8, normalized_y));
	ground = mix(ground, snow, smoothstep(0.8, 1.0, normalized_y));
	vec3 color = ground;

	if(show_normals) color = normal * 0.5 + 0.5;
	FragColor = vec4(color, 1.0);
}

