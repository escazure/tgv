#pragma once
#include "simple_generator.h"

class Terrain {
	public:
		float* vertices;
		unsigned int* indices;
		unsigned int size, isize;

		Terrain(){}
		Terrain(int _length, int _width, float _step_size){
			terrain_length = _length;
			terrain_width = _width;
			step_size = _step_size;
		}

		void generate(float (*fun)(float, float)){
			unsigned int vertex_count_x = std::floor(terrain_length/step_size);	
			unsigned int vertex_count_z = std::floor(terrain_width/step_size);

			vertices = generate_vertices(terrain_length, terrain_width, step_size, fun);
			size = vertex_count_x * vertex_count_z * 6;
			indices = generate_indices(terrain_length, terrain_width, step_size);
			isize = (vertex_count_x-1)*(vertex_count_z-1)*6;

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

			glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize * sizeof(unsigned int), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

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

};
