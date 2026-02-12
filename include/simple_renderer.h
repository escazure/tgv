#pragma once
#include <GLFW/glfw3.h>

//#include <glm/glm.h>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

struct Renderer {
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	Renderer(){}

	void init_renderer(float* vertices, unsigned int size, unsigned int* indices, unsigned int isize, bool is_color){
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize *sizeof(unsigned int), indices, GL_STATIC_DRAW);

		int stride = is_color ? 6 : 3;

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	
		if(is_color){
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3*sizeof(float)));
			glEnableVertexAttribArray(1);
		}
	}

	void render(unsigned int size){
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, 0);	
	}
};
