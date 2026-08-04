#include <cmath>
#include <cstdint>
#include <glm/glm.hpp>

inline float hash(float x, float z, unsigned int seed = 5527265){
	float h = sin(x * 12.564f + z * 18.851f) * seed;	
	return h - floor(h);
}

inline glm::vec2 hash22(const glm::vec2& p){
    glm::vec2 q = glm::fract(p * glm::vec2(0.1031f, 0.1030f));
    float d = glm::dot(q, glm::vec2(q.y + 33.33f, q.x + 33.33f));
    q = q + glm::vec2(d, d);
    
    glm::vec2 res = glm::fract(glm::vec2((q.x + q.y) * q.y, (q.x + q.y) * q.x));
    return res * 2.0f - 1.0f; 
}

inline uint32_t hash_seed(uint32_t seed){
	seed ^= seed >> 16;
	seed *= 0x85ebac88;
	seed ^= seed >> 13;
	seed *= 0x4ac849ba;
	seed ^= seed >> 16;
	return seed;
}

inline float lerp(float a, float b, float t){
	return a + t*(b-a);
}

inline float smooth(float t){
	return t*t*(3.0f-2.0f*t);
}

inline float bell_curve(float x, float z, float radius, float amplitude, int x_offset = 64, int z_offset = 64){
	float dx = x - x_offset;
	float dz = z - z_offset;
	float r2 = dx*dx+dz*dz;
	
	return exp(-r2 / (2*radius*radius)) * amplitude;
}

inline float value_noise(float x, float z){
	int x0 = floor(x);
	int z0 = floor(z);
	int x1 = x0+1;
	int z1 = z0+1;

	float sx = smooth(x - x0);
	float sz = smooth(z - z0);

	float v00 = hash(x0,z0);
	float v01 = hash(x0,z1);
	float v10 = hash(x1,z0);
	float v11 = hash(x1,z1);

	float ix0 = lerp(v00, v10, sx);
	float ix1 = lerp(v01, v11, sx);

	return lerp(ix0, ix1, sz) * 2.0f - 1.0f;
}

inline float perlin_noise(float x, float z){
	glm::vec2 uv = glm::vec2(x,z);
	glm::vec2 i = glm::floor(uv);
	glm::vec2 f = glm::fract(uv);

	glm::vec2 gradA = hash22(i);
	glm::vec2 gradB = hash22(i + glm::vec2(1.0, 0.0));
	glm::vec2 gradC = hash22(i + glm::vec2(0.0, 1.0));
	glm::vec2 gradD = hash22(i + glm::vec2(1.0, 1.0));

	float dotA = glm::dot(gradA, f);
	float dotB = glm::dot(gradB, f - glm::vec2(1.0, 0.0));
	float dotC = glm::dot(gradC, f - glm::vec2(0.0, 1.0));
	float dotD = glm::dot(gradD, f - glm::vec2(1.0, 1.0));

	glm::vec2 w = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
	
	float u1 = glm::mix(dotA, dotB, w.x);
	float u2 = glm::mix(dotC, dotD, w.x);

	return glm::mix(u1, u2, w.y);
}

inline glm::vec3 perlin_noise_with_derivatives(float x, float z){
	glm::vec2 uv = glm::vec2(x,z);
	glm::vec2 i = glm::floor(uv);
	glm::vec2 f = glm::fract(uv);

	glm::vec2 gradA = hash22(i);
	glm::vec2 gradB = hash22(i + glm::vec2(1.0, 0.0));
	glm::vec2 gradC = hash22(i + glm::vec2(0.0, 1.0));
	glm::vec2 gradD = hash22(i + glm::vec2(1.0, 1.0));

	float dotA = glm::dot(gradA, f);
	float dotB = glm::dot(gradB, f - glm::vec2(1.0, 0.0));
	float dotC = glm::dot(gradC, f - glm::vec2(0.0, 1.0));
	float dotD = glm::dot(gradD, f - glm::vec2(1.0, 1.0));

	glm::vec2 w = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
	glm::vec2 dw = f * f * (f * (f * 30.0f - 60.0f) + 30.0f);
	
	float u1 = glm::mix(dotA, dotB, w.x);
	float u2 = glm::mix(dotC, dotD, w.x);
	float val = glm::mix(u1, u2, w.y);

	glm::vec2 d = gradA + w.x * (gradB - gradA) + w.y * (gradC - gradA) + w.x * w.y * (gradA - gradB - gradC + gradD) +
                  dw * glm::vec2(w.y * (dotD - dotC - dotB + dotA) + dotB - dotA,
                                 w.x * (dotD - dotC - dotB + dotA) + dotC - dotA);

	return glm::vec3(val, d.x, d.y);
}

inline float fbm(float x, float z, float initFrequency = 1.0, float initAmplitude = 1.0, int octaves = 3){
	float total = 0.0f;
	float frequency = initFrequency;
	float amplitude = initAmplitude;

	for(int i = 0; i < octaves; i++){
		total += perlin_noise(x * frequency, z * frequency) * amplitude;

		frequency *= 2.0f;
		amplitude *= 0.5f;
	}

	return total;
}

inline float erosion_fbm(float x, float z, float erosionStrength = 0.5f, float initFrequency = 1.0f, float initAmplitude = 1.0f, int octaves = 3){
	float total = 0.0f;
	float frequency = initFrequency;
	float amplitude = initAmplitude;
	glm::vec2 derivative_total(0.0f);

	for(int i = 0; i < octaves; i++){
		glm::vec3 n = perlin_noise_with_derivatives(x * frequency, z * frequency);

		float noiseVal = n.x;
		glm::vec2 derivative = glm::vec2(n.y, n.z);
		derivative_total += derivative;

		float erosionFactor = 1.0f / (1.0f + glm::dot(derivative_total, derivative_total) * erosionStrength);
        total += noiseVal * amplitude * erosionFactor;

		frequency *= 2.0f;
		amplitude *= 0.5f;
	}

	return total;
}

inline float example(float x, float z, int seed = 1){
	float baseFrequency = 0.0005f;
	float baseAmplitude = 1000.0f;
	float baseLevel = 150.0f;

	uint32_t hashX = hash_seed(uint32_t(seed));
	uint32_t hashZ = hash_seed(hashX);

	float offsetX = float(hashX % 10000);
	float offsetZ = float(hashZ % 10000);

	float sx = x + offsetX;
	float sz = z + offsetZ;

	return erosion_fbm(sx, sz, 1.0f, baseFrequency, baseAmplitude) + baseLevel;
}
