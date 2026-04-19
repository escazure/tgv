#pragma once
#include <vector>
#include <iostream>

class Chunk{
	public:
		std::size_t vertex_count;
		std::size_t triangle_count;
		float max_height, min_height;

		Chunk(std::size_t _x_pos, std::size_t _z_pos, float _step_size, std::size_t _size, std::size_t _terrain_size){
			chunk_size = _size;
			step_size = _step_size;
			terrain_size = _terrain_size;
			x_pos = _x_pos;
			z_pos = _z_pos;
			vertex_count = 0;
			triangle_count = 0;
			max_height = 0.0f;
			min_height = 0.0f;
		}

		void generate(float (*fun)(float, float)){
			generate_vertices(fun);
			generate_indices();

			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glBindVertexArray(0);
		}

		void draw(){
			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
		
	private:
		std::vector<float> vertices;	
		std::vector<unsigned int> indices;
		unsigned int VAO, VBO, EBO;
		std::size_t chunk_size;		
		std::size_t terrain_size;
		std::size_t x_pos, z_pos;
		float step_size;

		void generate_vertices(float(*fun)(float, float)){
			int x0 = x_pos * chunk_size - (terrain_size * 0.5f);
			int z0 = z_pos * chunk_size - (terrain_size * 0.5f);
			std::size_t vertex_count_axis = std::floor(chunk_size/step_size)+1;
			vertices.resize(vertex_count_axis * vertex_count_axis * 3);
			vertex_count = vertex_count_axis * vertex_count_axis;

			int temp = 0;
			float max_y = std::numeric_limits<float>::min(), min_y = std::numeric_limits<float>::max();

			for(int i = 0; i < vertex_count_axis; i++){
				for(int j = 0; j < vertex_count_axis; j++){
					float world_x = x0 + i * step_size;
					float world_z = z0 + j * step_size;

					float y = fun(world_x, world_z);	

					vertices[temp] = world_x;	
					vertices[temp+1] = y;	
					vertices[temp+2] = world_z;	

					if(max_y < y) max_y = y;
					if(min_y > y) min_y = y;

					temp+=3;
				}
			}
			max_height = max_y;
			min_height = min_y;
		}

		void generate_indices(){
			unsigned int vertex_count_axis = std::floor(chunk_size/step_size)+1;
			indices.resize((vertex_count_axis-1)*(vertex_count_axis-1)*6);

			int temp = 0;
			for(int i = 0; i < vertex_count_axis-1; i++){
				for(int j = 0; j < vertex_count_axis-1; j++){
					indices[temp] = i*vertex_count_axis+j;	
					indices[temp+1] = i*vertex_count_axis+j+1;
					indices[temp+2] = (i+1)*vertex_count_axis+ j;
					
					indices[temp+3] = (i+1)*vertex_count_axis+ j;
					indices[temp+4] = i*vertex_count_axis+j+1;
					indices[temp+5] = (i+1)*vertex_count_axis+j+1;	
					temp+=6;

					triangle_count += 2;
				}
			} 
		}
};
