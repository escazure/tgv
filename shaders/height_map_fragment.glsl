#version 330 core

uniform float uWorldSize;
uniform int uSeed;

out float FragColor;
in vec2 uv;

vec2 hash(vec2 p){
	p = fract(p * vec2(0.1031, 0.1030));
    p += dot(p, p.yx + 33.33);
    return fract((p.xx + p.yy) * p.yx) * 2.0 - 1.0;
}

float hash21(ivec2 p, int seed) {
	uvec2 v = uvec2(p + ivec2(seed, seed * 1973));
	
	v = v * 1664525u + 1013904223u;
    v.x += v.y * 1664525u;
    v.y += v.x * 1664525u;
    v = v ^ (v >> 16u);
    v.x += v.y * 1664525u;
    v.y += v.x * 1664525u;
    v = v ^ (v >> 16u);

	return float(v.x) * (1.0 / 4294967295.0);
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

	return mix(u1, u2, w.y);
}

float fbm(vec2 uv){
	float value = 0.0;
	float amplitude = 1.0;
	for(int i = 0; i < 4; i++){
		value += amplitude * perlinNoise(uv);
		uv *= 2.0;
		amplitude *= 0.5;
	}
	return value;
}

void main(){
	float baseFrequency = 0.0005;
	float baseAmplitude = 1000.0;

	float offsetX = (hash21(ivec2(uSeed, 0), uSeed) - 0.5) * 1000.0;
	float offsetZ = (hash21(ivec2(0, uSeed), uSeed) - 0.5) * 1000.0;

	vec2 offsetUV = vec2(offsetX, offsetZ) + uv;

	float height = fbm(offsetUV * baseFrequency) * baseAmplitude;
	FragColor = height;
}
