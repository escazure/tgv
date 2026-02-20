#pragma once
#include <iostream>

inline float* generate_vertices(unsigned int length, unsigned int width, float step_size, float(*fun)(float, float)){
	int x0 = length/2 - length;
	int z0 = width/2 - width;
	unsigned int vertex_count_x = std::floor(length/step_size);
	unsigned int vertex_count_z = std::floor(width/step_size);
	float* vertices = new float[vertex_count_x * vertex_count_z * 6];	
	int temp = 0;
	float x = x0, z = z0, y;

	float max_y = std::numeric_limits<float>::min(), min_y = std::numeric_limits<float>::max();

	for(int i = 0; i < vertex_count_x; i++){
		z = z0;
		for(int j = 0; j < vertex_count_z; j++){
			y = fun(x, z);
			if(max_y < y) max_y = y;
			if(min_y > y) min_y = y;

			z+=step_size;
		}
		x+=step_size;
	}

	x = x0;

	for(int i = 0; i < vertex_count_x; i++){
		z = z0;
		for(int j = 0; j < vertex_count_z; j++){
			y = fun(x, z);	
			vertices[temp] = x;	
			vertices[temp+1] = y;	
			vertices[temp+2] = z;	

			float norm_y = (y - min_y)/(max_y - min_y);
			float r = 1.0f,g = 1.0f,b = 1.0f;
			if(norm_y < 0.3f){
				r = 0.8f; g = 0.8f, b = 0.5f;	
			}
			else if(norm_y < 0.5f){
				r = 0.1f; g = 0.7f, b = 0.1f;
			}
			else if(norm_y < 0.9f){
				r = 0.5f; g = 0.5f; b = 0.5f;
			}

			vertices[temp+3] = r;	
			vertices[temp+4] = g;	
			vertices[temp+5] = b;	
			temp+=6;

			z+=step_size;
		}
		x+=step_size;
	}

	return vertices;
}

inline float* generate_vertices_hm(unsigned int length, unsigned int width, float(*fun)(float, float)){
	float* vertices = new float[length * width * 6];	
	int temp = 0;
	for(int i = 0; i < length; i++){
		for(int j = 0; j < width; j++){
			vertices[temp] = j;	
			vertices[temp+1] = i;	
			vertices[temp+2] = 0;	

			vertices[temp+3] = 1 - 1/fun((float)j, (float)i);	
			vertices[temp+4] = 1 - 1/fun((float)j, (float)i);	
			vertices[temp+5] = 1 - 1/fun((float)j, (float)i);	
			temp+=6;
		}
	}

	return vertices;
}

inline unsigned int* generate_indices(unsigned int length, unsigned int width, float step_size){
	unsigned int vertex_count_x = std::floor(length/step_size);
	unsigned int vertex_count_z = std::floor(width/step_size);
	unsigned int* indices = new unsigned int[(vertex_count_x-1)*(vertex_count_z-1)*6];
	int temp = 0;
	for(int i = 0; i < vertex_count_x-1; i++){
		for(int j = 0; j < vertex_count_z-1; j++){
			indices[temp] = i*vertex_count_z+j;	
			indices[temp+1] = i*vertex_count_z+j+1;
			indices[temp+2] = (i+1)*vertex_count_z+ j;
			
			indices[temp+3] = (i+1)*vertex_count_z+ j;
			indices[temp+4] = i*vertex_count_z+j+1;
			indices[temp+5] = (i+1)*vertex_count_z+j+1;	
			temp+=6;
		}
	} 
	
	return indices;
}
