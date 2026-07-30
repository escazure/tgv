#version 330 core

in vec3 Normal;
in vec3 Position;
in vec3 WorldPos;
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

const float sandLevel = -20.0;
const float grassLevel = 250.0;
const float snowLevel = 420.0;

// TERRAIN COLORS //
const vec3 lushGrass = vec3(0.2, 0.45, 0.15);
const vec3 dryGrass = vec3(0.4, 0.45, 0.2);

const vec3 darkRock = vec3(0.3, 0.3, 0.25);
const vec3 lightRock = vec3(0.5, 0.45, 0.45);

const vec3 sand = vec3(0.7, 0.6, 0.4);
const vec3 snow = vec3(0.9, 0.9, 0.95);

float noise(vec2 position){
	return fract(sin(dot(position.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

float smoothNoise(vec2 position){
	vec2 i = floor(position);
	vec2 f = fract(position);

	float a = noise(i);
	float b = noise(i + vec2(1.0, 0.0));
	float c = noise(i + vec2(0.0, 1.0));
	float d = noise(i + vec2(1.0, 1.0));

	vec2 u = f * f * (3.0 - 2.0 * f);

	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

vec3 colorTerrain(float slope){
	vec2 warp = vec2(
		smoothNoise(WorldPos.xz * 0.02),
		smoothNoise(WorldPos.xz * (-0.05) + vec2(5.2, 1.3))
	) * 20.0;

    float noiseLarge = smoothNoise((WorldPos.xz + warp) * 0.005);
	vec2 p = (WorldPos.xz - warp) * 0.02;
	vec2 rotatedP = vec2(p.x * 0.7 - p.y * 0.7, p.x * 0.7 + p.y * 0.7);
	float octave1 = smoothNoise(p);
	float octave2 = smoothNoise(rotatedP * 2.1);
	float noiseDetail = octave1 * 0.6 + octave2 * 0.4;
    
    float noiseHeight = (smoothNoise(WorldPos.xz * 0.01) - 0.5) * 30.0;
    float noisyY = max(WorldPos.y + noiseHeight, 0.0); 

    vec3 varGrass = mix(lushGrass, dryGrass, noiseLarge * 0.7 + noiseDetail * 0.3);
    vec3 varRock = mix(darkRock, lightRock, noiseDetail);

    vec3 ground = mix(varRock, varGrass, slope);

    float sandFactor = smoothstep(sandLevel, 30.0, noisyY);
    float rockFactor = smoothstep(150.0, grassLevel + 100.0, noisyY);
    
    float snowHeightMask = smoothstep(370.0, snowLevel + 50.0, noisyY);
    float snowSlopeMask = smoothstep(0.3, 0.6, slope); 
    float snowFactor = snowHeightMask * snowSlopeMask;

    ground = mix(sand, ground, sandFactor);      
    ground = mix(ground, varRock, rockFactor);  
    ground = mix(ground, snow, snowFactor);    

    return ground;
}

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

	vec3 color = colorTerrain(slope);

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

