#pragma once

#include <dlfcn.h>
#include <fstream>

inline bool write_fun(const std::string& _fun, const std::string& _name){
	std::ofstream out("functions/" + _name + ".cpp");
	if(!out.good()){
		return false;
	}
	std::string fun_wrapper = "#include <cmath>\n"\
							   "#include \"simple_terrain_lib.h\"\n"\
							   "using namespace std;\nextern \"C\" float " + _name + "(float x, float z = 0){return " + _fun + ";}";
	out << fun_wrapper;
	return true;
}

inline bool compile_fun(const std::string& _name){
	std::string cmd = "g++ -Iinclude -shared -fPIC -O2 ./functions/" + _name + ".cpp -o ./functions/" + _name;
	int result = std::system(cmd.c_str());
	if(result != 0){
		return false;
	}
	return true;
}

inline bool get_fun_pointer(const std::string& _name, float (**_fun)(float,float), void** _handle){
	void* handle = dlopen(("./functions/"+_name).c_str(), RTLD_LAZY);
	if(!handle){
		return false;
	}
	*_fun = (float (*)(float,float))dlsym(handle, _name.c_str());
	const char* error = dlerror();
	if(error){
		dlclose(handle);
		return false;
	}
	*_handle = handle;
	return true;
}
