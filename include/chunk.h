#pragma once
#include <vector>

class Chunk{
public:
	std::size_t _vertexCount = 0;

	Chunk(std::size_t xPos, std::size_t zPos, float stepSize, std::size_t chunkSize, std::size_t terrainSize){
		_chunkSize = chunkSize;
		_stepSize = stepSize;
		_terrainSize = terrainSize;
		_xPos = xPos;
		_zPos = zPos;
	}

	~Chunk(){
		if(_VAO)
			glDeleteVertexArrays(1, &_VAO);
		if(_VBO)
			glDeleteBuffers(1, &_VBO);
	}

	void generate(unsigned int shaderEBO, std::size_t indexCount){
		_indexCount = indexCount;
		pGenerateVertices();

		glCreateVertexArrays(1, &_VAO);
		glCreateBuffers(1, &_VBO);

		glNamedBufferData(_VBO, _vertices.size() * sizeof(float), _vertices.data(), GL_STATIC_DRAW);

		glVertexArrayElementBuffer(_VAO, shaderEBO);

		glEnableVertexArrayAttrib(_VAO, 0);
		glVertexArrayAttribFormat(_VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(_VAO, 0, 0);
		glVertexArrayVertexBuffer(_VAO, 0, _VBO, 0, 2 * sizeof(float));

		_vertices.clear();
		_vertices.shrink_to_fit();
	}

	void draw() const {
		glBindVertexArray(_VAO);
		glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
		
private:
	unsigned int _VAO = 0, _VBO = 0;
	std::size_t _indexCount = 0;
	std::vector<float> _vertices;	

	std::size_t _chunkSize, _terrainSize;		
	std::size_t _xPos, _zPos;
	float _stepSize;

	void pGenerateVertices(){
		int x0 = _xPos * _chunkSize - (_terrainSize * 0.5f);
		int z0 = _zPos * _chunkSize - (_terrainSize * 0.5f);

		std::size_t axisVertexCount = std::size_t(_chunkSize / _stepSize) + 1;
		_vertexCount = axisVertexCount * axisVertexCount;
		_vertices.resize(_vertexCount * 2);

		std::size_t index = 0;
		for(std::size_t i = 0; i < axisVertexCount; i++){
			float world_x = x0 + i * _stepSize;
			for(std::size_t j = 0; j < axisVertexCount; j++){
				float world_z = z0 + j * _stepSize;
				_vertices[index++] = world_x;	
				_vertices[index++] = world_z;	
			}
		}
	}
};
