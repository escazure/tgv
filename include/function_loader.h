#pragma once
#include <dlfcn.h>
#include <fstream>
#include <iostream>

struct FunctionLoader{
	std::string function_buffer, function_name;
	std::string functions_path = PROJECT_FUNCTIONS_DIR;
	void* handle;
	bool writeFunction(){
		std::ofstream out(functions_path + "/" + function_name + ".cpp");
		if(!out.good()){
			return false;
		}
		std::string fun_wrapper = "#include <cmath>\n"\
							      "#include \"simple_terrain_lib.h\"\n"\
								  "using namespace std;\n"\
								  "extern \"C\" float " + function_name + "(float x, float z = 0){return " + function_buffer + ";}";
		out << fun_wrapper;
		out.close();
		return true;
	}
	
	bool compileFunction(){
		std::string include_path = PROJECT_INCLUDE_DIR;
		std::string functions_path = PROJECT_FUNCTIONS_DIR;

		std::string output = functions_path + "/" + function_name + ".so";
		std::string cmd = "g++ -I" + include_path + " -shared -fPIC -O2 " + functions_path + "/" + function_name + ".cpp -o " + output;

		int result = std::system(cmd.c_str());
		if(result != 0){
			std::cerr << "Compilation failed\n";
			return false;
		}
		return true;
	}

	bool getFunctionPointer(float (**_fun)(float,float)){
		if(handle){
			dlclose(handle);
			handle = nullptr;
		}
		std::string full_path = std::string(PROJECT_FUNCTIONS_DIR) + "/" + function_name + ".so";

		handle = dlopen(full_path.c_str(), RTLD_LAZY);
		if(!handle){
			std::cerr << "dlopen error: " << dlerror() << std::endl;
			return false;
		}

		dlerror();

		*_fun = (float (*)(float,float))dlsym(handle, function_name.c_str());

		const char* error = dlerror();
		if(error){
			std::cerr << error << std::endl;
			dlclose(handle);
			return false;
		}
		return true;
	}

	bool load(){
		if(!writeFunction())
			return false;
		if(!compileFunction())
			return false;
		return true;
	}

	void destroy(){
		if(handle){
			dlclose(handle);
			handle = nullptr;
		}
	}
};
