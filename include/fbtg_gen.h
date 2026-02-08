#pragma once

typedef unsigned int u32;

namespace generator {
	void generate_heightmap(float* vertex_arr, u32 width, u32 length, float (*fun)(float, float));
	void generate_vertices(float* vertex_arr, u32 width, u32 length, float (*fun)(float, float));
	inline void set_vertex(float* vertex_arr, u32& idx, float x, float y, float z);
}
