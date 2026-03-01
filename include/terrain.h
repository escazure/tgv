#pragma once
#include <chrono>
#include <iostream>

class Terrain {
	public:
		float* vertices;
		unsigned int* indices;
		unsigned int size, isize;

		float max, min, below_sea, gen_time;

		Terrain(){}
		Terrain(int _length, int _width, float _step_size){
			terrain_length = _length;
			terrain_width = _width;
			step_size = _step_size;
			max = 0.0f;
			min = 0.0f;
			below_sea = 0.0f;
			gen_time = 0.0f;
		}

		void generate(float (*fun)(float, float)){
			unsigned int vertex_count_x = std::floor(terrain_length/step_size);	
			unsigned int vertex_count_z = std::floor(terrain_width/step_size);
			
			auto start = std::chrono::high_resolution_clock::now();
			vertices = generate_vertices(terrain_length, terrain_width, step_size, fun);
			size = vertex_count_x * vertex_count_z * 3;
			indices = generate_indices(terrain_length, terrain_width, step_size);
			isize = (vertex_count_x-1)*(vertex_count_z-1)*6;
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> elapsed = end - start;
			gen_time = elapsed.count();

			below_sea = below_sea/(vertex_count_x*vertex_count_z) * 100;

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

			glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize * sizeof(unsigned int), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glBindVertexArray(0);
		}

		void draw(){
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, isize, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}

		~Terrain(){
			delete[] vertices;
			delete[] indices;
		}

	private:
		unsigned int VAO, VBO, EBO;
		int terrain_length, terrain_width;		
		float step_size;

		float* generate_vertices(unsigned int length, unsigned int width, float step_size, float(*fun)(float, float)){
			int x0 = length/2 - length;
			int z0 = width/2 - width;
			unsigned int vertex_count_x = std::floor(length/step_size);
			unsigned int vertex_count_z = std::floor(width/step_size);
			float* vertices = new float[vertex_count_x * vertex_count_z * 3];	
			int temp = 0;
			float x = x0, z = z0, y;

			float max_y = std::numeric_limits<float>::min(), min_y = std::numeric_limits<float>::max();

			for(int i = 0; i < vertex_count_x; i++){
				z = z0;
				for(int j = 0; j < vertex_count_z; j++){
					y = fun(x, z);	
					vertices[temp] = x;	
					vertices[temp+1] = y;	
					vertices[temp+2] = z;	

					if(y < 0.0f) below_sea++;
					if(max_y < y) max_y = y;
					if(min_y > y) min_y = y;

					temp+=3;

					z+=step_size;
				}
				x+=step_size;
			}

			max = max_y;
			min = min_y;
			return vertices;
		}

		unsigned int* generate_indices(unsigned int length, unsigned int width, float step_size){
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
};
