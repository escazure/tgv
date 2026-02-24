#pragma once

#include <dlfcn.h>
#include <fstream>

struct FunctionLoader{
	std::string function_buffer, function_name;
	void* handle;
	bool writeFunction(){
		std::ofstream out("functions/" + function_name + ".cpp");
		if(!out.good()){
			return false;
		}
		std::string fun_wrapper = "#include <cmath>\n"\
							   "#include \"simple_terrain_lib.h\"\n"\
								   "using namespace std;\nextern \"C\" float " + function_name + "(float x, float z = 0){return " + function_buffer + ";}";
		out << fun_wrapper;
		return true;
	}
	
	bool compileFunction(){
		std::string cmd = "g++ -Iinclude -shared -fPIC -O2 ./functions/" + function_name + ".cpp -o ./functions/" + function_name;
		int result = std::system(cmd.c_str());
		if(result != 0)
			return false;
		return true;
	}

	bool getFunctionPointer(float (**_fun)(float,float)){
		handle = dlopen(("./functions/"+function_name).c_str(), RTLD_LAZY);
		if(!handle){
			return false;
		}
		*_fun = (float (*)(float,float))dlsym(handle, function_name.c_str());
		const char* error = dlerror();
		if(error){
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
		
	}
};
