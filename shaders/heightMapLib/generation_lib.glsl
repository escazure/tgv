const float ONE_OVER_SQRT_TWO = 1.0 / sqrt(2.0);
const float ONE_OVER_MAX_UINT = 1.0 / 0xFFFFFFFFu;

uint uihash(uint v){
	v += 1664525u + uint(uSeed);
	v ^= v >> 12u;
	v *= 1664525u;
	v ^= v >> 16u;
	return v;	
}

float hash21(vec2 p){
	uvec2 v = floatBitsToUint(p);
	v.x += 1664525u + uint(uSeed);
	v ^= v.yx * 1664525u;
	v *= 1664525u;
	v ^= v >> 16u;

	uint floatBits = (v.x * v.y & 0x007FFFFFu) | 0x3F800000u;
	return uintBitsToFloat(floatBits) - 1.0;
}

vec2 hash22(vec2 p){
	uvec2 v = floatBitsToUint(p);
	v.x += 1664525u + uint(uSeed);
	v ^= v.yx * 1664525u;
	v *= 1664525u;
	v ^= v >> 16u;
	v ^= v.yx >> 5u;

	uvec2 floatBits = (v & 0x007FFFFFu) | 0x3F800000u;
	vec2 res = uintBitsToFloat(floatBits) - 1.0;
	return res;
}

// Smooth min by Inigo Quilez
float smin(float a, float b, float k){
    k *= 4.0;
    float h = max(k-abs(a-b), 0.0)/k;
    return min(a,b) - h*h*k*(1.0/4.0);
}

float bellCurve(vec2 pos, float radius, float amplitude){
	float r2 = pos.x * pos.x + pos.y * pos.y;
	return exp(-r2 / (2.0 * radius * radius)) * amplitude;
}

float valueNoise(vec2 uv){
	vec2 i = floor(uv);
	vec2 f = fract(uv);

	float a = hash21(i) * 2.0 - 1.0;
	float b = hash21(i + vec2(1.0, 0.0)) * 2.0 - 1.0;
	float c = hash21(i + vec2(0.0, 1.0)) * 2.0 - 1.0;
	float d = hash21(i + vec2(1.0, 1.0)) * 2.0 - 1.0;

	float u1 = mix(a, b, f.x);
	float u2 = mix(c, d, f.x);

	return mix(u1, u2, f.y);
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

	return mix(u1, u2, w.y);
}

vec3 perlinNoiseDerivatives(vec2 uv){
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
	vec2 dw = f * f * (f * (f * 30.0 - 60.0) + 30.0);

	float u1 = mix(dotA, dotB, w.x);
	float u2 = mix(dotC, dotD, w.x);
	float val = mix(u1, u2, w.y);

	vec2 g1 = mix(gradA, gradB, w.x);
	vec2 g2 = mix(gradC, gradD, w.x);
	vec2 gradInterpolation = mix(g1, g2, w.y);

	float dx = mix(dotB - dotA, dotD - dotC, w.y) * dw.x;
	float dy = mix(dotC - dotA, dotD - dotB, w.x) * dw.y;
	vec2 d = gradInterpolation + vec2(dx, dy);
	
	return vec3(val, d);
}

float voronoiNoise(vec2 uv, float cellSize){
	vec2 scaledUV = uv / cellSize;
	vec2 baseCell = floor(scaledUV);
	vec2 fracUV = fract(scaledUV);

    float minDist = 10.0;

	for(int x = -1; x <= 1; x++){
		for(int y = -1; y <= 1; y++){
			vec2 neighbor = vec2(float(x), float(y));
			vec2 point = hash22(baseCell + neighbor);
			vec2 diff = neighbor + point - fracUV;
			float dist = length(diff);

			minDist = min(minDist, dist);
		}
	}

	float normalizedDist = minDist * ONE_OVER_SQRT_TWO;
	return clamp(normalizedDist, 0.0, 1.0);
}

float fbm(vec2 uv, int octaves){
	float total = 0.0;
	float amplitude = 1.0;
	for(int i = 0; i < octaves; i++){
		total += amplitude * perlinNoise(uv);
		uv *= 2.0;
		amplitude *= 0.5;
	}
	return total;
}

float fbmErosion(vec2 uv, int octaves, float erosionStrength){
	float total = 0.0;
	float amplitude = 1.0;
	float frequency = 1.0;
	vec2 totalGradient = vec2(0.0);	
	for(int i = 0; i < octaves; i++){
		vec3 n = perlinNoiseDerivatives(uv);
		float h = n.x;
		totalGradient += n.yz * frequency;

		float erosionFactor = 1.0 / (1.0 + dot(totalGradient, totalGradient) * erosionStrength);
		total += h * erosionFactor * amplitude;

		uv *= 2.0;
		frequency *= 2.0;
		amplitude *= 0.5;
	}
	return total;
}

float fbmVoronoi(vec2 uv, int octaves, float cellSize){
	float total = 0.0;
	float amplitude = 1.0;
	for(int i = 0; i < octaves; i++){
		total += amplitude * voronoiNoise(uv, cellSize);
		uv *= 2.0;
		amplitude *= 0.5;
	}
	return total;
}

vec2 warpUV(vec2 uv, int octaves){
	float offset = fbm(uv, octaves);
	return vec2(uv.x + offset, uv.y - offset);
}

float mountains(vec2 worldPos, float frequency, float amplitude, float erosion, int octaves){
	vec2 warp = warpUV(worldPos, 8);
	float height = fbmErosion(warp * frequency, octaves, erosion) * amplitude;
	return height;
}

float plains(vec2 worldPos, float frequency, float amplitude){
	vec2 warp;
	warp.x = perlinNoise(worldPos * 0.001);
	warp.y = perlinNoise(worldPos * 0.001 + 123.4);

	vec2 p = worldPos + warp * 300.0;
	float large = perlinNoise(p * frequency);
	float detail = perlinNoise(p * frequency * 2.0);

	return (large * 0.8 + detail * 0.2) * amplitude;
}

/*
float desert(vec2 uv, ...){}
float platos(vec2 uv, ...){}
*/
