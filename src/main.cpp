#include "core.h"

#define PLAIN_LENGTH 2
#define PLAIN_WIDTH 2

int main(){
	/*
	std::string test_fun, test_name;
	std::cout << "Provide function: \n";
	std::getline(std::cin, test_fun);

	std::cout << "Provide function name: \n";
	std::getline(std::cin, test_name);

	if(!write_fun(test_fun, test_name)){
		std::cerr << "Failed to open/create file" << std::endl;
		return 1;
	}
    
	if(!compile_fun(test_name)){
		std::cerr << "Failed to compile file" << std::endl;
		return 1;
	}

	float (*fun)(float,float);
	void* handle;

	if(!get_fun_pointer(test_name, &fun, &handle)){
		std::cerr << "Failed to get function pointer" << std::endl;
		return 1;
	}

	float* vertices = generate_vertices(PLAIN_LENGTH, PLAIN_WIDTH, fun);	
	unsigned int size = PLAIN_LENGTH * PLAIN_WIDTH * 6;

	for(int i = 0; i < size; i++){
		if(i%6==0) std::cout << std::endl;
		std::cout << vertices[i] << " ";
	}
*/
	unsigned int* indices = new unsigned int[]{
		0, 1, 2,			
	};

	float* vertices = new float[]{
		-1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 1.0, 0.0, 0.0,
	};
	unsigned int size = 18;

	GLFWwindow* window = init(vertices, size, indices, 6);
	run(window, vertices, size);
	shutdown(window);

	delete[] vertices;
	delete[] indices;
}
