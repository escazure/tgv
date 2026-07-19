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
			generate_normals(fun);

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

		void generate_normals(float(*fun)(float, float)) {
    		unsigned int vertex_count_axis = std::floor(chunk_size / step_size) + 1;
    		normals.resize(vertex_count * 3, 0.0f);

    		int x0 = x_pos * chunk_size - (terrain_size * 0.5f);
    		int z0 = z_pos * chunk_size - (terrain_size * 0.5f);

    		for (int i = 0; i < vertex_count_axis; i++) {
        		for (int j = 0; j < vertex_count_axis; j++) {
            		int current_idx = (i * vertex_count_axis + j) * 3;

            		float L_world_x = x0 + (i - 1) * step_size;
            		float R_world_x = x0 + (i + 1) * step_size;
            		float B_world_z = z0 + (j - 1) * step_size;
            		float F_world_z = z0 + (j + 1) * step_size;

            		float current_world_x = x0 + i * step_size;
            		float current_world_z = z0 + j * step_size;
	
    		        float hL = fun(L_world_x, current_world_z); 
        		    float hR = fun(R_world_x, current_world_z); 
            		float hB = fun(current_world_x, B_world_z);
            		float hF = fun(current_world_x, F_world_z); 

            		float tan_x_x = 2.0f * step_size;
            		float tan_x_y = hR - hL;
            		float tan_x_z = 0.0f;
		
            		float tan_z_x = 0.0f;
            		float tan_z_y = hF - hB;
            		float tan_z_z = 2.0f * step_size;

            		float n_x = tan_z_y * tan_x_z - tan_z_z * tan_x_y;
            		float n_y = tan_z_z * tan_x_x - tan_z_x * tan_x_z;
            		float n_z = tan_z_x * tan_x_y - tan_z_y * tan_x_x;

            		float len = n_x * n_x + n_y * n_y + n_z * n_z;
            		if (len > 1e-10f) {
                		float inv_len = 1.0f / std::sqrt(len);
                		n_x *= inv_len;
                		n_y *= inv_len;
                		n_z *= inv_len;
            		}

            		normals[current_idx]     = n_x;
            		normals[current_idx + 1] = n_y;
            		normals[current_idx + 2] = n_z;
        		}
    		}
		}
};
