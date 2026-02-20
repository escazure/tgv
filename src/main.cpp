#include "core.h"

#define PLAIN_LENGTH 512
#define PLAIN_WIDTH 512
#define STEP_SIZE 0.5f

int main(){
	std::cout << "Menu:\n- random(x,z,seed) - generates random fractional value\n" \
						 "- value_noise(x,z) - value noise\n" \
						 "- fbm(x,z,octave) - fractal brownian motion\n" \
						 "- example(x,z,seed) - predefined terrain function\n";
	
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

	unsigned int vertex_count_x = std::floor(PLAIN_LENGTH/STEP_SIZE);
	unsigned int vertex_count_z = std::floor(PLAIN_WIDTH/STEP_SIZE);

	float* vertices = generate_vertices(PLAIN_LENGTH, PLAIN_WIDTH, STEP_SIZE , fun);	
	unsigned int size = vertex_count_x * vertex_count_z * 6;
	unsigned int* indices = generate_indices(PLAIN_LENGTH, PLAIN_WIDTH, STEP_SIZE);
	unsigned int isize = (vertex_count_x-1)*(vertex_count_z-1)*6;

	GLFWwindow* window = init(vertices, size, indices, isize);

	run(window, vertices, size);
	shutdown(window);

	delete[] vertices;
	delete[] indices;
}
