#version 330 core

in vec3 Normal;
in vec3 Position;
in float normalized_y;
in vec4 FragPosLightSpace;

in mat4 LightSpaceMatrix;

out vec4 FragColor;

uniform bool show_normals;
uniform bool calculate_lighting;
uniform vec3 lightDir;

uniform sampler2DShadow shadowMap;

uniform float min_bias;
uniform float max_bias;

const vec3 up = vec3(0.0, 1.0, 0.0);
const vec3 lightCol = vec3(1.0);

const vec3 grass = vec3(0.2, 0.8, 0.2);
const vec3 rock = vec3(0.5, 0.5, 0.5);
const vec3 sand = vec3(0.7, 0.7, 0.5);
const vec3 snow = vec3(1.0, 1.0, 1.0);

vec3 calculateLight(vec3 fragmentColor, vec3 normal, vec3 lightDirection, vec3 lightColor, float ambientStrength, float diffuseStrength, float shadow){
	vec3 ambient = fragmentColor * lightColor * ambientStrength;

	vec3 fragmentToLight = normalize(-lightDirection);
	float diff = max(dot(normal, fragmentToLight), 0.0);
	vec3 diffuse = fragmentColor * lightColor * diff * diffuseStrength;

	return (ambient + (1.0 - shadow) * diffuse);
}

float calculateShadows(vec4 fragPosLightSpace, vec3 normal, vec3 lightDirection){
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	if(projCoords.z > 1.0) return 0.0;

	float bias = max(max_bias * (1.0 - dot(normal, -lightDirection)), min_bias);
	float shadow = texture(shadowMap, vec3(projCoords.xy, projCoords.z - bias));

	return 1.0 - shadow;
}

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

	if(calculate_lighting){
		vec3 fragmentToLight = normalize(-lightDir);
		float shadow = 1.0;

		if(dot(normal, fragmentToLight) > 0.0){
			shadow = calculateShadows(FragPosLightSpace, normal, lightDir);
		}

		FragColor = vec4(calculateLight(color, normal, lightDir, lightCol, 0.2, 0.8, shadow), 1.0);
	}
}

