#version 330 core

in vec3 Normal;
in vec3 Position;
in vec3 WorldPos;
in vec4 FragPosLightSpace;
in mat4 LightSpaceMatrix;

out vec4 FragColor;

uniform sampler2DShadow shadowMap;

uniform bool show_normals;
uniform bool calculate_lighting;

uniform float min_bias;
uniform float max_bias;

uniform vec3 lightDir;

const vec3 up = vec3(0.0, 1.0, 0.0);
const vec3 lightCol = vec3(1.0);

const float sandLevel = -20.0;
const float grassLevel = 250.0;
const float rockLevel = 350.0;
const float snowLevel = 420.0;

// TERRAIN COLORS //
const vec3 tree = vec3(0.16, 0.27, 0.23);
const vec3 grass = vec3(0.45, 0.65, 0.3);

const vec3 darkRock = vec3(0.3, 0.3, 0.25);
const vec3 lightRock = vec3(0.5, 0.45, 0.45);

const vec3 sand = vec3(0.7, 0.6, 0.4);
const vec3 snow = vec3(0.9, 0.9, 0.95);

float hash(vec2 p){
	return fract(sin(dot(p.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec2 getGradientVector(vec2 point){
	float hashValue = hash(point);
	hashValue *= 6.283185; 
	return vec2(cos(hashValue), sin(hashValue));
}

float perlinNoise(vec2 uv){
	vec2 cornerA = floor(uv);
	vec2 cornerB = cornerA + vec2(1.0, 0.0);
	vec2 cornerC = cornerA + vec2(0.0, 1.0);
	vec2 cornerD = cornerA + vec2(1.0, 1.0);
	vec2 f = fract(uv);

	vec2 gradA = getGradientVector(cornerA);
	vec2 gradB = getGradientVector(cornerB);
	vec2 gradC = getGradientVector(cornerC);
	vec2 gradD = getGradientVector(cornerD);

	vec2 offsetA = uv - cornerA;
	vec2 offsetB = uv - cornerB;
	vec2 offsetC = uv - cornerC;
	vec2 offsetD = uv - cornerD;

	float dotA = dot(gradA, offsetA);
	float dotB = dot(gradB, offsetB);
	float dotC = dot(gradC, offsetC);
	float dotD = dot(gradD, offsetD);

	vec2 w = 6 * (f * f * f * f * f) - 15 * (f * f * f * f) + 10 * (f * f * f);
	float u1 = dotA + w.x * (dotB - dotA);
	float u2 = dotC + w.x * (dotD - dotC);

	return (u1 + w.y * (u2 - u1)) * 0.5 + 0.5;
}

vec3 colorTerrain(float slope, vec3 normal){
	vec2 uv = WorldPos.xz;

	float noiseLarge = perlinNoise(uv * 0.005);
    float noiseDetail = perlinNoise(uv * 0.02);
	float noiseMicro = perlinNoise(uv * 0.08);
	float combinedNoise = noiseLarge * 0.7 + noiseDetail * 0.3;

	float warpedY = WorldPos.y + (noiseLarge * 100.0 + noiseDetail * 50.0);

	vec3 sandMask = sand;

	float grassNoise = smoothstep(0.4, 0.75, combinedNoise);
	vec3 grassMask = mix(tree, grass, grassNoise);

	float rockNoise = smoothstep(0.5, 0.75, noiseDetail);
	vec3 rockMask = mix(darkRock, lightRock, rockNoise);

	vec3 snowMask = mix(snow, vec3(1.0), noiseMicro * 0.3);

	float grassWeight = smoothstep(sandLevel, sandLevel + 20.0, warpedY);
	float rockWeight = smoothstep(rockLevel, rockLevel + 40.0, warpedY);
	float snowWeight = smoothstep(snowLevel, snowLevel + 30.0, warpedY);

	vec3 ground = sandMask;
	ground = mix(ground, grassMask, grassWeight);
	ground = mix(ground, rockMask, rockWeight);

	float cliffFactor = smoothstep(0.6, 0.4, slope);
	ground = mix(ground, rockMask, cliffFactor);

	float snowSlopeFactor = smoothstep(0.45, 0.8, slope);
	float snowPatchiness = snowWeight * snowSlopeFactor - noiseMicro * 0.4;
	float snowCoverage = smoothstep(0.15, 0.7, snowPatchiness) * 0.85;
	ground = mix(ground, snowMask, snowCoverage);

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

	vec3 color = colorTerrain(slope, normal);

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

