const float SQRT_TWO = sqrt(2.0);
const float ONE_OVER_SQRT_TWO = 1.0 / sqrt(2.0);
const float ONE_OVER_MAX_UINT = 1.0 / 0xFFFFFFFFu;
const float PI = 3.1415926535;

float hash11(float p){
	uint v = floatBitsToUint(p);
	v += 1664525u + uint(uSeed);
	v ^= v >> 12u;
	v *= 1664525u;
	v ^= v >> 16u;

	uint floatBits = (v & 0x007FFFFFu) | 0x3F800000u;
	return uintBitsToFloat(floatBits) - 1.0;	
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

float bellCurve(vec2 p, float radius, float amplitude){
	float r2 = p.x * p.x + p.y * p.y;
	return exp(-r2 / (2.0 * radius * radius)) * amplitude;
}

float valueNoise(vec2 p){
	vec2 i = floor(p);
	vec2 f = fract(p);

	float a = hash21(i) * 2.0 - 1.0;
	float b = hash21(i + vec2(1.0, 0.0)) * 2.0 - 1.0;
	float c = hash21(i + vec2(0.0, 1.0)) * 2.0 - 1.0;
	float d = hash21(i + vec2(1.0, 1.0)) * 2.0 - 1.0;

	vec2 w = f * f * (f * -2.0 + 3.0);

	float u1 = mix(a, b, w.x);
	float u2 = mix(c, d, w.x);

	return mix(u1, u2, w.y);
}

float perlinNoise(vec2 p){
	vec2 i = floor(p);
	vec2 f = fract(p);

	float A = hash21(i) * 2.0 * PI;
	float B = hash21(i + vec2(1.0, 0.0)) * 2.0 * PI;
	float C = hash21(i + vec2(0.0, 1.0)) * 2.0 * PI;
	float D = hash21(i + vec2(1.0, 1.0)) * 2.0 * PI;
	vec2 gradA = vec2(cos(A), sin(A));
	vec2 gradB = vec2(cos(B), sin(B));
	vec2 gradC = vec2(cos(C), sin(C));
	vec2 gradD = vec2(cos(D), sin(D));

	float dotA = dot(gradA, f);
	float dotB = dot(gradB, f - vec2(1.0, 0.0));
	float dotC = dot(gradC, f - vec2(0.0, 1.0));
	float dotD = dot(gradD, f - vec2(1.0, 1.0));

	vec2 w = f * f * f * (f * (f * 6.0 - 15.0) + 10.0);

	float u1 = mix(dotA, dotB, w.x);
	float u2 = mix(dotC, dotD, w.x);

	return mix(u1, u2, w.y) * SQRT_TWO;
}

vec3 perlinNoiseDerivatives(vec2 p){
	vec2 i = floor(p);
	vec2 f = fract(p);

	float A = hash21(i) * 2.0 * PI;
	float B = hash21(i + vec2(1.0, 0.0)) * 2.0 * PI;
	float C = hash21(i + vec2(0.0, 1.0)) * 2.0 * PI;
	float D = hash21(i + vec2(1.0, 1.0)) * 2.0 * PI;
	vec2 gradA = vec2(cos(A), sin(A));
	vec2 gradB = vec2(cos(B), sin(B));
	vec2 gradC = vec2(cos(C), sin(C));
	vec2 gradD = vec2(cos(D), sin(D));

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
	
	return vec3(val, d) * SQRT_TWO;
}

float voronoiNoise(vec2 p){
	vec2 i = floor(p);
	vec2 f = fract(p);

    float minDist = 10.0;

	for(int x = -1; x <= 1; x++){
		for(int y = -1; y <= 1; y++){
			vec2 neighbor = vec2(float(x), float(y));
			vec2 point = hash22(i + neighbor);
			vec2 diff = neighbor + point - f;
			float dist = length(diff);

			minDist = min(minDist, dist);
		}
	}

	return minDist * ONE_OVER_SQRT_TWO;
}

float valueFBM(vec2 p, float persistence, float lacunarity, int octaves){
	float total = 0.0;
	float amplitude = 1.0;
	float amplitudeTotal = 0.0;
	for(int i = 0; i < octaves; i++){
		total += valueNoise(p) * amplitude;
		amplitudeTotal += amplitude;

		p *= lacunarity;
		amplitude *= persistence;
	}
	return total / amplitudeTotal;
}

float perlinFBM(vec2 p, float persistence, float lacunarity, int octaves){
	float total = 0.0;
	float amplitude = 1.0;
	float amplitudeTotal = 0.0;
	for(int i = 0; i < octaves; i++){
		total += perlinNoise(p) * amplitude;
		amplitudeTotal += amplitude;

		p *= lacunarity;
		amplitude *= persistence;
	}
	return total / amplitudeTotal;
}

float voronoiFBM(vec2 p, float persistence, float lacunarity, int octaves){
	float total = 0.0;
	float amplitude = 1.0;
	float amplitudeTotal = 0.0;
	for(int i = 0; i < octaves; i++){
		total += voronoiNoise(p) * amplitude;
		amplitudeTotal += amplitude;

		p *= lacunarity;
		amplitude *= persistence;
	}
	return total / amplitudeTotal;
}

float gradientFBM(vec2 p, float persistence, float lacunarity, float gradientStrength, int octaves){
	float total = 0.0;
	float amplitude = 1.0;
	float amplitudeTotal = 0.0;
	float frequency = 1.0;
	vec2 gradientTotal = vec2(0.0);	
	for(int i = 0; i < octaves; i++){
		vec3 n = perlinNoiseDerivatives(p);
		gradientTotal += n.yz * frequency * amplitude;
		amplitudeTotal += amplitude;

		vec2 normalizedGradient = gradientTotal / amplitudeTotal;
		float gradientFactor = 1.0 / (1.0 + dot(normalizedGradient, normalizedGradient) * gradientStrength);
		total += n.x * gradientFactor * amplitude;

		p *= lacunarity;
		frequency *= lacunarity;
		amplitude *= persistence;
	}
	return total / amplitudeTotal;
}

float ridgeFBM(vec2 p, float persistence, float lacunarity, int octaves){
	float total = 0.0;
	float amplitude = 1.0;
	float amplitudeTotal = 0.0;
	float weight = 1.0;
	for(int i = 0; i < octaves; i++){
		float r = 1.0 - abs(perlinNoise(p));
		r *= r;
		r *= weight;

		total += r * amplitude;
		amplitudeTotal += amplitude;

		weight = clamp(r * 2.0, 0.0, 1.0);

		p *= lacunarity;
		amplitude *= persistence;
	}
	return total / amplitudeTotal * 2.0 - 1.0;
}

vec2 domainWarp(vec2 p, float strength){
	float x = perlinNoise(p);
	float y = perlinNoise(p + vec2(123.4, 48.9));
	return vec2(x, y) * strength;
}

float terrainMountains(vec2 p, float gradientStrength, float ridgeWeight, int octaves){
	p += domainWarp(p * 0.1, 2.0);

	float mass = gradientFBM(p, 0.5, 2.0, gradientStrength, octaves);
	float ridges = ridgeFBM(p, 0.5, 2.0, octaves);

	float ridgeMask = smoothstep(-0.3, 0.5, mass);
	ridges *= ridgeMask;
	
	float height = mass * (1.0 - ridgeWeight) + ridges * ridgeWeight;
	return height * 0.5 + 0.5;
}

float terrainPlains(vec2 p, int octaves){
	vec2 warp = domainWarp(p * 0.1, 2.0);
	return perlinFBM(p + warp, 0.3, 2.0, octaves) * 0.5 + 0.5;
}

float terrainHills(vec2 p, float persistence, int octaves){
	vec2 warp = domainWarp(p * 0.5, 0.5);
    float height = perlinFBM(p + warp, persistence, 2.0, octaves);
    return height * 0.5 + 0.5;
}

float terrainRugged(vec2 p, float ridgeWeight, float gradientStrength, int octaves){
	p += domainWarp(p * 0.2, 1.0);

	float mass = gradientFBM(p, 0.5, 2.0, gradientStrength, octaves);
	float ridges = ridgeFBM(p * 1.5, 0.5, 2.0, octaves);

	float height = mix(mass, ridges, ridgeWeight);
	return height * 0.5 + 0.5;
}

float terrainPlateaus(vec2 p, float lowEdge, float highEdge, int detailLevels, int levels){
	float n = perlinFBM(worldPos, 0.5, 2.0, detailLevels);
	n = n * 0.5 + 0.5;

	float scaled = n * float(levels);
	float level = floor(scaled);
	float transition = fract(scaled);
	transition = smoothstep(lowEdge, highEdge, transition);
	
	float height = (level + transition) / levels;
	return height;
}

float terrainCanyons(vec2 p, float lowEdge, float highEdge, float depth){
	p += domainWarp(p * 0.2, 0.5);
	float path = abs(perlinNoise(p));
	float canyon = 1.0 - smoothstep(lowEdge, highEdge, path);
	return -canyon * depth;
}

float terrainDunes(vec2 p, float direction, float duneNoiseStrength, float crestNoiseStrength, float warpStrength, float detailStrength){
    vec2 dir = vec2(cos(direction), sin(direction));
    vec2 warpedP = p + domainWarp(p * 0.12, warpStrength);

    float along = dot(warpedP, dir);
    float across = dot(warpedP, vec2(-dir.y, dir.x));

    float phase = across + perlinFBM(p * 0.12, 0.5, 2.0, 3) * 0.8;
    float seed = floor(phase / (2.0 * PI)) * 0.37;

    float dune = 1.0 - abs(fract(phase / (2.0 * PI)) * 2.0 - 1.0);

    float duneHeightNoise = perlinFBM(vec2(seed, 12.73), 0.5, 2.0, 3);
    float crestNoise= perlinFBM(vec2(seed, along * 0.15), 0.5, 2.0, 3);

    float height = 1.0 + duneHeightNoise * duneNoiseStrength + crestNoise * crestNoiseStrength;
    float detail = perlinFBM(warpedP * 2.0, 0.5, 2.0, 4);

    return dune * height + detail * detailStrength * 0.05;
}


float terrainWeight(vec2 control, vec2 center, float sharpness){
	vec2 d = control - center;
	return exp(-dot(d, d) * sharpness);
}

float example(vec2 p, vec3 terrainBias, float classSharpness){
	vec2 control = vec2(
			perlinNoise(p * 0.4),
			perlinNoise(p * 0.4 + vec2(123.4, 59.8))
	);

	float wPlains = terrainWeight(control, vec2(-0.5, -0.3), classSharpness);
	float wPlateaus = terrainWeight(control, vec2(0.4, -0.5), classSharpness);
	float wMountains = terrainWeight(control, vec2(0.4, 0.6), classSharpness);
	float wTotal = wPlains * terrainBias.x + wPlateaus * terrainBias.y + wMountains * terrainBias.z;

	wPlains /= wTotal;
	wPlateaus /= wTotal;
	wMountains /= wTotal;

	float plains = terrainPlains(p, 3);
	float plateaus = terrainPlateaus(p, 0.35, 0.65, 5, 5);
	float mountains = terrainMountains(p, 0.5, 0.35, 6);

	float height = plains * wPlains + plateaus * wPlateaus + mountains * wMountains;
	return height;
}
