#pragma once

#include <fstream>
#include <vector>
#include <array>

namespace emit::obj_exporter {
	bool begin(const char* file_name);
	bool write_vertex_vec(const std::vector<float>& vertex_data);
	bool write_face_vec(const std::vector<int>& face_data);
	bool write_vertex_arr(const float* vertex_data, std::size_t n);
	bool write_face_arr(const int* face_data, std::size_t n);
	void end();
}
