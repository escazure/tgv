#version 460 core

in vec3 WorldPos;
in vec2 uv;
out vec4 FragColor;

uniform bool show_normals;
uniform bool calculate_lighting;

uniform float min_bias;
uniform float max_bias;

uniform vec3 lightDir;

uniform sampler2D uNormalMap;
uniform sampler2D uShadowMap;

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

vec3 colorTerrain(float slope){
	vec2 uv = WorldPos.xz;

    float noiseLarge  = perlinNoise(uv * 0.005);
    float noiseDetail = perlinNoise(uv * 0.02);
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
        float noiseMicro = perlinNoise(uv * 0.08);
        vec3 snowMask = mix(snow, vec3(1.0), noiseMicro * 0.3);
        float snowPatchiness = maxSnowPotential - noiseMicro * 0.4;
        float snowCoverage = smoothstep(0.15, 0.7, snowPatchiness) * 0.85;

        ground = mix(ground, snowMask, snowCoverage);
    }

    return ground;
}

vec3 calculateLight(vec3 fragmentColor, vec3 normal, vec3 fragToLight, vec3 lightColor, float shadow){
	float skyFactor = normal.y * 0.5 + 0.5;
	vec3 skyColor = vec3(0.25, 0.3, 0.4);
	vec3 groundColor = vec3(0.1, 0.08, 0.05);
	vec3 ambient = fragmentColor * mix(groundColor, skyColor, skyFactor);

	float diff = max(dot(normal, fragToLight), 0.0);
	vec3 diffuse = fragmentColor * lightColor * diff * 0.8;

	return ambient + shadow * diffuse;
}

void main(){
	vec3 normal = texture(uNormalMap, uv).rgb * 2.0 - 1.0;
	float slope = 1.0 - normal.y;
	vec3 color = colorTerrain(slope);

	if(show_normals){
		FragColor = vec4(normal * 0.5 + 0.5, 1.0);
		return;
	}

	if(calculate_lighting){
		vec3 fragToLight = normalize(-lightDir);
		float shadow = texture(uShadowMap, uv).r;

		color = calculateLight(color, normal, fragToLight, lightCol, shadow);
	}
	float shadow = texture(uShadowMap, uv).r;

	FragColor = vec4(color, 1.0);
}

