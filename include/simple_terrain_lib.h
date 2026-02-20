#include <cmath>

inline float random(float x, float z, unsigned int seed = 5527265){
	float h = sin(x * 12.564f + z * 18.851f) * seed;	
	return h - floor(h);
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

	float v00 = random(x0,z0);
	float v01 = random(x0,z1);
	float v10 = random(x1,z0);
	float v11 = random(x1,z1);

	float ix0 = lerp(v00, v10, sx);
	float ix1 = lerp(v01, v11, sx);

	return lerp(ix0, ix1, sz) * 2.0f - 1.0f;
}

inline float fbm(float x, float z, int octaves = 4){
	float total = 0.0f;
	float amplitude = 1.0f;
	float frequency = 1.0f;

	for(int i = 0; i < octaves; i++){
		total += value_noise(x * frequency, z * frequency) * amplitude;

		frequency *= 2.0f;
		amplitude *= 0.5f;
	}

	return total/2.0-1.0;
}

inline float example(float x, float z, int seed = 1000){
	return fbm(x*0.00047+seed/20, z*0.00042+seed/30, 5)*200+std::pow(fbm(x*0.0041+seed*2, z*0.0039-seed*4, 6),2)*100+fbm(x*0.04+seed, z*0.056-seed, 3)*10;
}
