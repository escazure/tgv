#pragma once
#include <chrono>
#include <cstdint>
#include <vector>
#include "chunk.h"

class Terrain {
	public:
		int _size;
		float _stepSize;
		std::size_t _chunkSize;
		std::size_t _vertexCount = 0;
		std::size_t _triangleCount = 0;

		Terrain(){}
		Terrain(std::size_t size, float stepSize, std::size_t chunkSize){
			_size = size;
			_stepSize = stepSize;
			_chunkSize = chunkSize;
		
			std::size_t chunksCount = size / chunkSize;
			_chunks.reserve(chunksCount * chunksCount);

			for(std::size_t x = 0; x < chunksCount; x++){
				for(std::size_t z = 0; z < chunksCount; z++){
					_chunks.emplace_back(x, z, stepSize, chunkSize, size);
				}
			}
		}

		~Terrain(){
			if(_sharedEBO) glDeleteBuffers(1, &_sharedEBO);
		}
		
		void generate(){
			pGenerateSharedEBO();

			for(std::size_t i = 0; i < _chunks.size(); i++){
				_chunks[i].generate(_sharedEBO, _sharedIndices.size());
				_vertexCount += _chunks[i]._vertexCount;
				_triangleCount += _sharedIndices.size() / 3;
			}	

			_sharedIndices.clear();
			_sharedIndices.shrink_to_fit();
		}
		
		void draw() const {
			for(std::size_t i = 0; i < _chunks.size(); i++){
				_chunks[i].draw();
			}
		}

	private:
		unsigned int _sharedEBO = 0;
		std::vector<unsigned int> _sharedIndices;
		std::vector<Chunk> _chunks;

		void pGenerateSharedEBO(){
			std::size_t axisVertexCount = std::size_t(_chunkSize / _stepSize) + 1;
			std::size_t quadCount = (axisVertexCount - 1) * (axisVertexCount - 1);

			_sharedIndices.resize(quadCount * 6);

			std::size_t index = 0;
			for(std::size_t i = 0; i < axisVertexCount - 1; i++){
				for(std::size_t j = 0; j < axisVertexCount - 1; j++){
					std::size_t row1 = std::size_t(i * axisVertexCount + j);
					std::size_t row2 = std::size_t((i + 1) * axisVertexCount + j);

					_sharedIndices[index++] = row1;
					_sharedIndices[index++] = row1 + 1;
					_sharedIndices[index++] = row2;

					_sharedIndices[index++] = row2;
					_sharedIndices[index++] = row1 + 1;
					_sharedIndices[index++] = row2 + 1;
				}
			}

			glCreateBuffers(1, &_sharedEBO);
			glNamedBufferData(_sharedEBO, _sharedIndices.size() * sizeof(unsigned int), _sharedIndices.data(), GL_STATIC_DRAW);
		}
};
