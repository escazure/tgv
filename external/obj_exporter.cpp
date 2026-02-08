#include "emit.hpp"

namespace {
	std::ofstream file;
}

namespace emit::obj_exporter {

bool begin(const char* file_name){
	if(file.is_open()) return false;
	file.open(file_name);

	if(!file){
		return false;
	}

	file << "# " << file_name << "\n\n";
	return true;
}

bool write_vertex_vec(const std::vector<float>& vertex_data){
	if(!file.is_open() || vertex_data.size() % 3 != 0){
		return false;	
	}

	for(std::size_t i = 0; i < vertex_data.size(); i+=3){
		file << "v " << vertex_data[i] << " " << vertex_data[i+1] << " " << vertex_data[i+2] << "\n";
	}
	file << "\n";
	return true;
}

bool write_face_vec(const std::vector<int>& face_data){
	if(!file.is_open() || face_data.size() % 3 != 0){
		return false;
	}

	for(std::size_t i = 0; i < face_data.size(); i+=3){
		file << "f " << face_data[i] << " " << face_data[i+1] << " " << face_data[i+2] << "\n";
	}

	file << "\n";
	return true;
}

bool write_vertex_arr(const float* vertex_data, std::size_t n){
	if(!file.is_open() || n % 3 != 0){
		return false;
	}	

	for(std::size_t i = 0; i < n; i+=3){
		file << "v " << vertex_data[i] << " " << vertex_data[i+1] << " " << vertex_data[i+2] << "\n";
	}

	file << "\n";
	return true;
}

bool write_face_arr(const int* face_data, std::size_t n){
	if(!file.is_open() || n % 3 != 0){
		return false;
	}

	for(std::size_t i = 0; i < n; i+=3){
		file << "f " << face_data[i] << " " << face_data[i+1] << " " << face_data[i+2] << "\n";
	}

	file << "\n";
	return true;
}

void end(){
	if(file.is_open()){
		file.close();
	}
}

}
