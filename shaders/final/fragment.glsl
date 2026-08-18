#version 460 core
layout (binding = 1) uniform sampler2D uNormalMap;
layout (binding = 2) uniform sampler2D uShadowMap;

in vec3 WorldPos;
in vec2 uv;
out vec4 FragColor;

uniform bool uShowNormals;
uniform bool uCalculateLighting;

uniform int uTextureMethod;
uniform float uChunkSize;
uniform float uMinHeight;
uniform float uMaxHeight;

uniform vec3 uLightDir;
const vec3 up = vec3(0.0, 1.0, 0.0);
const vec3 lightCol = vec3(1.0);

const float sandLevel = -20.0;
const float grassLevel = 250.0;
const float rockLevel = 350.0;
const float snowLevel = 420.0;

#define DEFAULT_TEXTURE 0
#define ALTITUDE_TEXTURE 1
#define GRID_TEXTURE 2

// TERRAIN COLORS //
const vec3 tree = vec3(0.16, 0.27, 0.23);
const vec3 grass = vec3(0.45, 0.65, 0.3);

const vec3 darkRock = vec3(0.3, 0.3, 0.25);
const vec3 lightRock = vec3(0.5, 0.45, 0.45);

const vec3 sand = vec3(0.7, 0.6, 0.4);
const vec3 snow = vec3(0.9, 0.9, 0.95);

vec2 hash(vec2 p){
	p = fract(p * vec2(0.1031, 0.1030));
    p += dot(p, p.yx + 33.33);
    return fract((p.xx + p.yy) * p.yx) * 2.0 - 1.0;
}

float perlinNoise(vec2 uv){
	vec2 i = floor(uv);
	vec2 f = fract(uv);

	vec2 gradA = hash(i);
	vec2 gradB = hash(i + vec2(1.0, 0.0));
	vec2 gradC = hash(i + vec2(0.0, 1.0));
	vec2 gradD = hash(i + vec2(1.0, 1.0));

	float dotA = dot(gradA, f);
	float dotB = dot(gradB, f - vec2(1.0, 0.0));
	float dotC = dot(gradC, f - vec2(0.0, 1.0));
	float dotD = dot(gradD, f - vec2(1.0, 1.0));

	vec2 w = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

	float u1 = mix(dotA, dotB, w.x);
	float u2 = mix(dotC, dotD, w.x);

	return mix(u1, u2, w.y) * 0.5 + 0.5;
}

vec3 textureTerrainDefault(float slope){
	vec2 wp = WorldPos.xz;

    float noiseLarge  = perlinNoise(wp * 0.005);
    float noiseDetail = perlinNoise(wp * 0.02);
    float combinedNoise = noiseLarge * 0.7 + noiseDetail * 0.3;
    float warpedY = WorldPos.y + noiseLarge * 100.0 + noiseDetail * 50.0;

	float grassWeight = smoothstep(sandLevel, sandLevel + 20.0, warpedY);
    float rockWeight = smoothstep(rockLevel, rockLevel + 40.0, warpedY);
    float snowWeight = smoothstep(snowLevel, snowLevel + 30.0, warpedY);
    float cliffFactor = smoothstep(0.4, 0.6, slope);

    float grassNoise = smoothstep(0.4, 0.75, combinedNoise);
    vec3 grassMask = mix(tree, grass, grassNoise);
    vec3 ground = mix(sand, grassMask, grassWeight);

    float totalRockFactor = max(rockWeight, cliffFactor);
    if (totalRockFactor > 0.0) {
        float rockNoise = smoothstep(0.5, 0.75, noiseDetail);
        vec3 rockMask = mix(darkRock, lightRock, rockNoise);
        ground = mix(ground, rockMask, totalRockFactor);
    }

    float snowSlopeFactor = smoothstep(0.5, 0.15, slope);
    float maxSnowPotential = snowWeight * snowSlopeFactor;

    if(maxSnowPotential > 0.05){
        float noiseMicro = perlinNoise(wp * 0.08);
        vec3 snowMask = mix(snow, vec3(1.0), noiseMicro * 0.3);
        float snowPatchiness = maxSnowPotential - noiseMicro * 0.4;
        float snowCoverage = smoothstep(0.15, 0.7, snowPatchiness) * 0.85;

        ground = mix(ground, snowMask, snowCoverage);
    }

    return ground;
}

vec3 textureTerrainAltitude(float height, float minHeight, float maxHeight){
	float t = (height - minHeight) / (maxHeight - minHeight);

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

	float val = height / lineInterval;
	float delta = fwidth(val);
	float distToLine = abs(fract(val - 0.5) - 0.5);
	float lineFactor = distToLine / delta;
	float mask = smoothstep(lineWidth, lineWidth - 1.0, lineFactor);

	return mix(color, lineColor, mask);
}

vec3 textureTerrainGrid(float chunkSize){
	vec2 wp = WorldPos.xz;
	vec3 baseColor = vec3(0.5);
	vec3 lineColor = vec3(0.0);

	float lineWidth = 0.5;
	vec2 coord = wp / chunkSize;
	
	vec2 delta = fwidth(coord);
	vec2 grid = abs(fract(coord - 0.5) - 0.5);

	vec2 lineCoverage = vec2(0.0);
	if(delta.x > 0.0){
		float halfWidth = lineWidth / chunkSize;
		vec2 lineMin = (grid - halfWidth) / delta;
		vec2 lineMax = (grid + halfWidth) / delta;
		lineCoverage = clamp(lineMax, 0.0, 1.0) - clamp(lineMin, 0.0, 1.0);
	}

	float lineFactor = max(lineCoverage.x, lineCoverage.y);
	return mix(baseColor, lineColor, lineFactor);
}

vec3 calculateLight(vec3 fragmentColor, vec3 normal, vec3 fragToLight, vec3 lightColor, float shadow){
	vec3 ambient = fragmentColor * 0.2;

	float diff = max(dot(normal, fragToLight), 0.0);
	vec3 diffuse = fragmentColor * lightColor * diff * 0.8;

	return ambient + shadow * diffuse;
}

void main(){
	vec3 normal = texture(uNormalMap, uv).rgb * 2.0 - 1.0;
	float slope = 1.0 - normal.y;
	vec3 color;

	if(uTextureMethod == DEFAULT_TEXTURE) color = textureTerrainDefault(slope);
	else if(uTextureMethod == ALTITUDE_TEXTURE) color = textureTerrainAltitude(WorldPos.y, uMinHeight, uMaxHeight);
	else if(uTextureMethod == GRID_TEXTURE) color = textureTerrainGrid(uChunkSize);

	if(uShowNormals){
		FragColor = vec4(normal * 0.5 + 0.5, 1.0);
		return;
	}

	if(uCalculateLighting){
		vec3 fragToLight = normalize(-uLightDir);
		float shadow = texture(uShadowMap, uv).r;

		color = calculateLight(color, normal, fragToLight, lightCol, shadow);
	}
	float shadow = texture(uShadowMap, uv).r;

	FragColor = vec4(color, 1.0);
}

