vec2 hash22(vec2 p){
	p = fract(p * vec2(0.1031, 0.1030));
    p += dot(p, p.yx + 33.33);
    return fract((p.xx + p.yy) * p.yx) * 2.0 - 1.0;
}

uint hashSeed(uint seed) {
    seed ^= seed >> 16u;
    seed *= 0x85ebac88u;
    seed ^= seed >> 13u;
    seed *= 0x4ac849bau;
    seed ^= seed >> 16u;
    return seed;
}

vec2 shiftUV(vec2 uv, float shiftSize, int seed){
	uint hashX = hashSeed(uint(seed));
	uint hashZ = hashSeed(hashX);

	float normalizedX = (float(hashX) / 4294967295.0) * 2.0 - 1.0;
	float normalizedZ = (float(hashZ) / 4294967295.0) * 2.0 - 1.0;

	return vec2(normalizedX, normalizedZ) * shiftSize + uv;
}

float bellCurve(vec2 uv, float radius, float amplitude){
	float r2 = uv.x * uv.x + uv.y * uv.y;
	return exp(-r2 / (2.0 * radius * radius)) * amplitude;
}

float perlinNoise(vec2 uv){
	vec2 i = floor(uv);
	vec2 f = fract(uv);

	vec2 gradA = hash22(i);
	vec2 gradB = hash22(i + vec2(1.0, 0.0));
	vec2 gradC = hash22(i + vec2(0.0, 1.0));
	vec2 gradD = hash22(i + vec2(1.0, 1.0));

	float dotA = dot(gradA, f);
	float dotB = dot(gradB, f - vec2(1.0, 0.0));
	float dotC = dot(gradC, f - vec2(0.0, 1.0));
	float dotD = dot(gradD, f - vec2(1.0, 1.0));

	vec2 w = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

	float u1 = mix(dotA, dotB, w.x);
	float u2 = mix(dotC, dotD, w.x);

	return mix(u1, u2, w.y);
}

float fbm(vec2 uv, int octaves){
	float value = 0.0;
	float amplitude = 1.0;
	for(int i = 0; i < octaves; i++){
		value += amplitude * perlinNoise(uv);
		uv *= 2.0;
		amplitude *= 0.5;
	}
	return value;
}

float example(vec2 uv){
	const float baseFrequency = 0.0005;
	const float baseAmplitude = 1000.0;
	const float shiftSize = 500000.0;

	vec2 offsetUV = shiftUV(uv, shiftSize, uSeed);

	return fbm(offsetUV * baseFrequency, 3) * baseAmplitude;
}
