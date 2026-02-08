#include "fbtg_gen.h"

inline void generator::set_vertex(float* vertex_arr, u32& idx, float x, float y, float z){
	vertex_arr[idx++] = x;
	vertex_arr[idx++] = y;
	vertex_arr[idx++] = z;
}

void generator::generate_heightmap(float* vertex_arr, u32 width, u32 length, float (*fun)(float, float)){
	u32 idx = 0;
	for(u32 i = 0; i < width-1; i++){
		for(u32 j = 0; j < length-1; j++){
			set_vertex(vertex_arr, idx, i, j, fun(i,j));
			set_vertex(vertex_arr, idx, i+1, j, fun(i+1,j));
			set_vertex(vertex_arr, idx, i,j+1, fun(i, j+1));

			set_vertex(vertex_arr, idx, i,j+1, fun(i, j+1));
			set_vertex(vertex_arr, idx, i+1,j, fun(i+1, j));
			set_vertex(vertex_arr, idx, i+1,j+1, fun(i+1, j+1));

		}
	}
}

void generator::generate_vertices(float* vertex_arr, u32 width, u32 length, float (*fun)(float, float)){
	u32 idx = 0;
	for(u32 i = 0; i < width-1; i++){
		for(u32 j = 0; j < length-1; j++){
			set_vertex(vertex_arr, idx, i,fun(i, j),j);
			set_vertex(vertex_arr, idx, i+1,fun(i+1, j),j);
			set_vertex(vertex_arr, idx, i,fun(i, j+1),j+1);

			set_vertex(vertex_arr, idx, i,fun(i, j+1),j+1);
			set_vertex(vertex_arr, idx, i+1,fun(i+1, j),j);
			set_vertex(vertex_arr, idx, i+1,fun(i+1, j+1),j+1);
		}
	}
}
