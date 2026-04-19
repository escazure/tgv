#pragma once
#include <chrono>
#include <iostream>
#include <vector>
#include "chunk.h"

class Terrain {
	public:
		float max_height, min_height, gen_time;
		std::size_t vertex_count;
		std::size_t triangle_count;

		Terrain(){}
		Terrain(std::size_t _size, float _step_size, std::size_t _chunk_size){
			terrain_length = _size;
			terrain_width = _size;
			step_size = _step_size;
			max_height = 0.0f;
			min_height = 0.0f;
			gen_time = 0.0f;
			vertex_count = 0;
			triangle_count = 0;
		
			std::size_t chunks_count = _size/_chunk_size;
			chunks.reserve(chunks_count * chunks_count);
			std::cout << "Resized to " << chunks_count << ":" << chunks_count << " chunks\n";

			for(std::size_t x = 0; x < chunks_count; x++){
				for(std::size_t z = 0; z < chunks_count; z++){
					chunks.emplace_back(x,z,step_size,_chunk_size, terrain_length);
				}
			}
		}

		void generate(float (*fun)(float, float)){
			std::cout << "Started chunks generation, for " << chunks.size() << " chunks\n";
			auto start_time = std::chrono::high_resolution_clock::now();
			for(std::size_t i = 0; i < chunks.size(); i++){
				chunks[i].generate(fun);
				vertex_count += chunks[i].vertex_count;
				triangle_count += chunks[i].triangle_count;
				if(max_height < chunks[i].max_height) max_height = chunks[i].max_height;
				if(min_height > chunks[i].min_height) min_height = chunks[i].min_height;
			}	
			auto end_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> duration = end_time - start_time;
			gen_time = duration.count();
			std::cout << "Finished chunks generation, for " << chunks.size() << " chunks\n";
		}
		
		void draw(){
			for(std::size_t i = 0; i < chunks.size(); i++){
				chunks[i].draw();
			}
		}

	private:
		int terrain_length, terrain_width;		
		float step_size;
		std::vector<Chunk> chunks;
};
