#include "fbtg_core.h"
#include "plugin_system.hpp"
#include "emit.hpp"

namespace eo = emit::obj_exporter;

constexpr u32 width = 33, length = 33;

constexpr int VERTEX_ARR_SIZE = (width-1)*(length-1)*18;
constexpr int FACES_ARR_SIZE = (width-1)*(length-1)*6;

float vertices_temp[VERTEX_ARR_SIZE];
int faces[FACES_ARR_SIZE];

int main(){  
	std::string test_fun, test_name;
	std::cout << "Provide function: \n";
	std::getline(std::cin, test_fun);

	std::cout << "Provide function name: \n";
	std::getline(std::cin, test_name);

	if(!plugin_system::write_fun(test_fun, test_name)){
		error_callback(-1, ("Failed to open/create file: " + test_name).c_str());
		return 1;
	}
    
	if(!plugin_system::compile_fun(test_name)){
		error_callback(-2, ("Failed to compile file: " + test_name).c_str());
		return 1;
	}

	float (*fun)(float,float);
	void* handle;

	if(!plugin_system::get_fun_pointer(test_name, &fun, &handle)){
		error_callback(-5, "Failed to get function pointer");
		return 1;
	}

	generator::generate_heightmap(vertices_temp, width, length, fun);
    GLFWwindow* window = init_subsystems(vertices_temp, width, length);
	if(!window){
		std::cout << "Something during initialization gone wrong, see logs\n";
		return -1;
	}

	for(std::size_t i = 0; i < FACES_ARR_SIZE; i++){
		faces[i] = i+1;
	}
	
	if(!eo::begin("test.obj")) std::cout << "Failed to open file\n";
	if(!eo::write_vertex_arr(vertices_temp, VERTEX_ARR_SIZE)) std::cout << "Failed to write vertex data\n";
	if(!eo::write_face_arr(faces, FACES_ARR_SIZE)) std::cout << "Failed to write faced data\n";
	eo::end();

    run(window);
    shutdown(&handle, window);
}
