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
			generate_normals();

			std::vector<float> vertex_data;
			vertex_data.resize(vertex_count * 6);
			for(std::size_t i = 0, j = 0;  i < vertices.size(); i += 3, j += 6){
				vertex_data[j] = vertices[i];	
				vertex_data[j+1] = vertices[i+1];	
				vertex_data[j+2] = vertices[i+2];	

				vertex_data[j+3] = normals[i];	
				vertex_data[j+4] = normals[i+1];	
				vertex_data[j+5] = normals[i+2];	
			}


			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glGenBuffers(1, &VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glGenBuffers(1, &EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

			glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
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
		std::vector<float> normals;
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
			float max_y = std::numeric_limits<float>::lowest(), min_y = std::numeric_limits<float>::max();

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

		void generate_normals(){
			normals.resize(vertex_count * 3, 0.0f);	

			for(std::size_t i = 0; i < indices.size(); i += 3){
				unsigned int i0 = indices[i] * 3;	
				unsigned int i1 = indices[i+1] * 3;	
				unsigned int i2 = indices[i+2] * 3;	

				float v0_x = vertices[i0];						
				float v0_y = vertices[i0+1];						
				float v0_z = vertices[i0+2];						

				float v1_x = vertices[i1];						
				float v1_y = vertices[i1+1];						
				float v1_z = vertices[i1+2];						

				float v2_x = vertices[i2];						
				float v2_y = vertices[i2+1];						
				float v2_z = vertices[i2+2];						

				float e0_x = v1_x - v0_x; 
				float e0_y = v1_y - v0_y; 
				float e0_z = v1_z - v0_z; 

				float e1_x = v2_x - v0_x; 
				float e1_y = v2_y - v0_y; 
				float e1_z = v2_z - v0_z; 

				float n_x = e0_y*e1_z - e0_z*e1_y;	
				float n_y = e0_z*e1_x - e0_x*e1_z;	
				float n_z = e0_x*e1_y - e0_y*e1_x;	

				normals[i0] += n_x;
				normals[i0+1] += n_y;
				normals[i0+2] += n_z;

				normals[i1] += n_x;
				normals[i1+1] += n_y;
				normals[i1+2] += n_z;

				normals[i2] += n_x;
				normals[i2+1] += n_y;
				normals[i2+2] += n_z;
			}

			for(std::size_t i = 0; i < normals.size(); i += 3){
				float n_x = normals[i];
				float n_y = normals[i+1];
				float n_z = normals[i+2];

				float len = n_x * n_x + n_y * n_y + n_z * n_z;
				if(len > 1e-10f){
					float inv_len = 1.0f / std::sqrt(len);
					n_x *= inv_len;
					n_y *= inv_len;
					n_z *= inv_len;
				}

				normals[i] = n_x;
				normals[i+1] = n_y;
				normals[i+2] = n_z;
			}
		}
};
