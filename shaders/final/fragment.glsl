#version 460 core
layout (binding = 0) uniform sampler2D uHeightMap;
layout (binding = 1) uniform sampler2D uNormalMap;
layout (binding = 2) uniform sampler2D uShadowMap;

layout (location = 0) out vec4 FragColor;

in vec3 WorldPos;
in vec2 uv;

uniform bool uShowNormals;
uniform bool uCalculateLighting;

uniform int uTextureMethod;
uniform float uChunkSize;
uniform float uMinHeight;
uniform float uMaxHeight;

uniform vec3 uLightDir;
const vec3 up = vec3(0.0, 1.0, 0.0);

#define MOUNTAINS_TEXTURE 0
#define PLAINS_TEXTURE 1
#define DESERT_TEXTURE 2
#define POLAR_TEXTURE 3
#define ALTITUDE_TEXTURE 4
#define GREYSCALE_TEXTURE 5
#define GRID_TEXTURE 6

// TERRAIN COLORS //
const vec3 DARK_GREEN = vec3(0.2, 0.29, 0.22);
const vec3 LIGHT_GREEN = vec3(0.51, 0.59, 0.35);

const vec3 DARK_BROWN = vec3(0.4, 0.32, 0.23);
const vec3 LIGHT_BROWN = vec3(0.59, 0.48, 0.34);

const vec3 DARK_GREY = vec3(0.32, 0.31, 0.26);
const vec3 LIGHT_GREY = vec3(0.54, 0.52, 0.47);

const vec3 DARK_SAND = vec3(0.55, 0.43, 0.29);
const vec3 LIGHT_SAND = vec3(0.78, 0.7, 0.55);

const vec3 LIGHT_BLUE = vec3(0.72, 0.86, 0.91);
const vec3 DARK_BLUE = vec3(0.35, 0.58, 0.68);

const vec3 WHITE = vec3(1.0);
const vec3 BLUISH_WHITE = vec3(0.85, 0.85, 0.88);

const vec3 TERRACOTA = vec3(0.62, 0.34, 0.24);
const vec3 BEIGE = vec3(0.66, 0.58, 0.44);

vec2 hash22(vec2 p){
	uvec2 v = floatBitsToUint(p);
	v.x += 1664525u;
	v ^= v.yx * 1664525u;
	v *= 1664525u;
	v ^= v >> 16u;
	v ^= v.yx >> 5u;

	uvec2 floatBits = (v & 0x007FFFFFu) | 0x3F800000u;
	vec2 res = uintBitsToFloat(floatBits) - 1.0;
	return res;
}

float perlinNoise(vec2 uv){
	vec2 i = floor(uv);
	vec2 f = fract(uv);

	vec2 gradA = normalize(hash22(i) * 2.0 - 1.0);
	vec2 gradB = normalize(hash22(i + vec2(1.0, 0.0)) * 2.0 - 1.0);
	vec2 gradC = normalize(hash22(i + vec2(0.0, 1.0)) * 2.0 - 1.0);
	vec2 gradD = normalize(hash22(i + vec2(1.0, 1.0)) * 2.0 - 1.0);

	float dotA = dot(gradA, f);
	float dotB = dot(gradB, f - vec2(1.0, 0.0));
	float dotC = dot(gradC, f - vec2(0.0, 1.0));
	float dotD = dot(gradD, f - vec2(1.0, 1.0));

	vec2 w = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

	float u1 = mix(dotA, dotB, w.x);
	float u2 = mix(dotC, dotD, w.x);

	return mix(u1, u2, w.y) * 0.5 + 0.5;
}

vec3 textureTerrainMountains(float slope){
	vec2 wp = WorldPos.xz;

    float noiseLarge  = perlinNoise(wp * 0.005);
    float noiseDetail = perlinNoise(wp * 0.015);
    float noiseCombined = noiseLarge * 0.7 + noiseDetail * 0.3;

    float warpedY = WorldPos.y + (noiseCombined - 0.5) * 50.0;
	float heightPercent = clamp(
			(warpedY - uMinHeight) / (uMaxHeight - uMinHeight)
			, 0.0, 1.0
	);

	float grassVariation = noiseLarge * 0.3 + 0.375;	
	float rockVariation = noiseLarge * 0.3 + (noiseDetail - 0.5) * 0.1;
	float snowVariation = noiseDetail * 0.3;

    vec3 grassMask = mix(DARK_GREEN, LIGHT_GREEN, grassVariation);
    vec3 rockMask = mix(DARK_GREY, LIGHT_GREY, rockVariation);
	vec3 snowMask = mix(BLUISH_WHITE, WHITE, snowVariation);

	float rockHeightFactor = smoothstep(0.4, 0.65, heightPercent);
	float rockSlopeFactor = smoothstep(0.35, 0.6, slope);
	float rockFactor = max(rockHeightFactor, rockSlopeFactor);

	float snowFactor = smoothstep(0.72, 0.9, heightPercent);
	snowFactor *= 1.0 - smoothstep(0.2, 0.5, slope);

	vec3 ground = mix(grassMask, rockMask, rockFactor);
    ground = mix(ground, snowMask, snowFactor);

    return ground;
}

vec3 textureTerrainPlains(float slope){
	vec2 wp = WorldPos.xz;

    float noiseLarge = perlinNoise(wp * 0.001);
	float noiseMedium = perlinNoise(wp * 0.004);
    float noiseDetail = perlinNoise(wp * 0.02);

    float noiseWarped = noiseMedium + (noiseLarge - 0.5) * 0.25;

	float grassVariation = noiseLarge * 0.3 + (noiseDetail - 0.5) * 0.08 + 0.25;
	float dirtVariation = noiseMedium;
	float rockVariation = noiseLarge * 0.3 + (noiseDetail - 0.5) * 0.1;

	vec3 grassMask = mix(DARK_GREEN, LIGHT_GREEN, grassVariation);
	vec3 dirtMask = mix(DARK_BROWN, LIGHT_BROWN, dirtVariation);
    vec3 rockMask = mix(DARK_GREY, LIGHT_GREY, rockVariation);

	float cliffThreshold = mix(0.42, 0.58, noiseLarge);
	float grassOvergrowth = smoothstep(0.35, 0.55, noiseLarge);
	grassOvergrowth *= 1.0 - smoothstep(0.35, 0.6, slope);
    float rockFactor = smoothstep(cliffThreshold - 0.1, cliffThreshold + 0.1, slope);
	rockFactor *= 1.0 - grassOvergrowth * 0.12;

	float dirtFactor = smoothstep(0.48, 0.68, noiseWarped);
	dirtFactor *= 1.0 - rockFactor;
	dirtFactor *= 0.5;

	vec3 ground = mix(grassMask, dirtMask, dirtFactor);
	ground = mix(ground, rockMask, rockFactor);

	return ground;
}

vec3 textureTerrainDesert(float slope){
	vec2 wp = WorldPos.xz;	

	float noiseLarge = perlinNoise(wp * 0.001);
	float noiseDetail = perlinNoise(wp * 0.012);

	float sandVariation = noiseLarge * 0.25 + (noiseDetail - 0.5) * 0.1 + 0.35;
	float rockVariation = noiseLarge * 0.3 + 0.4;

	vec3 sandMask = mix(DARK_SAND, LIGHT_SAND, sandVariation);
	vec3 terracotaMask = mix(TERRACOTA, DARK_BROWN, rockVariation);

	float cliffThreshold = mix(0.42, 0.58, noiseLarge) + (noiseDetail - 0.5) * 0.1;
	float terracotaFactor = smoothstep(cliffThreshold - 0.1, cliffThreshold + 0.1, slope);
	
	vec3 ground = mix(sandMask, terracotaMask, terracotaFactor);
	return ground;
}

vec3 textureTerrainPolar(float slope){
	vec2 wp = WorldPos.xz;	

	float noiseLarge = perlinNoise(wp * 0.001);
	float noiseDetail = perlinNoise(wp * 0.012);

	float snowVariation = noiseLarge * 0.25 + (noiseDetail - 0.5) * 0.1 + 0.35;
	float iceVariation = noiseLarge * 0.2 + 0.4;

	vec3 snowMask = mix(BLUISH_WHITE, WHITE, snowVariation);
	vec3 iceMask = mix(DARK_BLUE, LIGHT_BLUE, iceVariation);

	float cliffThreshold = mix(0.42, 0.58, noiseLarge) + (noiseDetail - 0.5) * 0.1;
	float iceFactor = smoothstep(cliffThreshold - 0.1, cliffThreshold + 0.1, slope);
	
	vec3 ground = mix(snowMask, iceMask, iceFactor);
	return ground;
}

vec3 textureTerrainAltitude(){
	float t = (WorldPos.y - uMinHeight) / (uMaxHeight - uMinHeight);

	vec3 color;

	const vec3 deepBlue = vec3(0.02, 0.08, 0.35);
	const vec3 cyanBlue = vec3(0.2, 0.6, 0.85);
	const vec3 green = vec3(0.2, 0.65, 0.25);
	const vec3 yellow = vec3(0.95, 0.85, 0.25);
	const vec3 orange = vec3(0.9, 0.45, 0.1);
	const vec3 red = vec3(0.75, 0.1, 0.1);

	if(t < 0.1) color = mix(deepBlue, cyanBlue, t / 0.1);
	else if(t < 0.35) color = mix(cyanBlue, green, (t - 0.1) / 0.25);
	else if(t < 0.65) color = mix(green, yellow, (t - 0.35) / 0.30);
	else if(t < 0.85) color = mix(yellow, orange, (t - 0.65) / 0.20);
	else color = mix(orange, red, (t - 0.85) / 0.15);

	vec3 lineColor = vec3(0.0);
	float lineInterval = 75.0;
	float lineWidth = 1.0;

	float val = WorldPos.y / lineInterval;
	float delta = fwidth(val);
	float distToLine = abs(fract(val - 0.5) - 0.5);
	float lineFactor = distToLine / delta;
	float mask = smoothstep(lineWidth, lineWidth - 1.0, lineFactor);

	return mix(color, lineColor, mask);
}

vec3 textureTerrainGreyscale(){
	float t = (WorldPos.y - uMinHeight) / (uMaxHeight - uMinHeight);
	return vec3(t);
}

vec3 textureTerrainGrid(){
	vec2 wp = WorldPos.xz;
	vec3 baseColor = vec3(0.5);
	vec3 lineColor = vec3(0.0);

	float lineWidth = 0.5;
	vec2 coord = wp / uChunkSize;
	
	vec2 delta = fwidth(coord);
	vec2 grid = abs(fract(coord - 0.5) - 0.5);

	vec2 lineCoverage = vec2(0.0);
	if(delta.x > 0.0){
		float halfWidth = lineWidth / uChunkSize;
		vec2 lineMin = (grid - halfWidth) / delta;
		vec2 lineMax = (grid + halfWidth) / delta;
		lineCoverage = clamp(lineMax, 0.0, 1.0) - clamp(lineMin, 0.0, 1.0);
	}

	float lineFactor = max(lineCoverage.x, lineCoverage.y);
	return mix(baseColor, lineColor, lineFactor);
}

vec3 calculateLight(vec3 fragmentColor, vec3 normal, vec3 fragToLight, float shadow){
	vec3 ambient = fragmentColor * 0.2;

	float diff = max(dot(normal, fragToLight), 0.0);
	vec3 diffuse = fragmentColor * diff * 0.8;

	return ambient + shadow * diffuse;
}

void main(){
	vec3 normal = texture(uNormalMap, uv).rgb * 2.0 - 1.0;
	float slope = 1.0 - normal.y;
	vec3 color;

	if(uTextureMethod == MOUNTAINS_TEXTURE) color = textureTerrainMountains(slope);
	if(uTextureMethod == PLAINS_TEXTURE) color = textureTerrainPlains(slope);
	if(uTextureMethod == DESERT_TEXTURE) color = textureTerrainDesert(slope);
	if(uTextureMethod == POLAR_TEXTURE) color = textureTerrainPolar(slope);
	else if(uTextureMethod == ALTITUDE_TEXTURE) color = textureTerrainAltitude();
	else if(uTextureMethod == GREYSCALE_TEXTURE) color = textureTerrainGreyscale();
	else if(uTextureMethod == GRID_TEXTURE) color = textureTerrainGrid();

	if(uShowNormals){
		FragColor = vec4(normal * 0.5 + 0.5, 1.0);
		return;
	}

	if(uCalculateLighting){
		vec3 fragToLight = normalize(-uLightDir);
		float shadow = texture(uShadowMap, uv).r;

		color = calculateLight(color, normal, fragToLight, shadow);
	}
	float shadow = texture(uShadowMap, uv).r;

	FragColor = vec4(color, 1.0);
}

