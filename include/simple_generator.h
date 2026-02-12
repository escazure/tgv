#pragma once

inline float* generate_vertices(unsigned int length, unsigned int width, float(*fun)(float, float)){
	float* vertices = new float[length * width * 6];	
	for(int i = 0; i < length; i++){
		for(int j = 0; j < width; j++){
			vertices[j] = i;	
			vertices[j+1] = fun((float)j,(float)i);	
			vertices[j+2] = j;	

			vertices[j+3] = 1;	
			vertices[j+4] = 0;	
			vertices[j+5] = 0;	
		}
	}

	return vertices;
}
