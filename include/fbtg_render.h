#pragma once

#include "plugin_system.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../external/gl3w.h"
#include <GLFW/glfw3.h>

namespace render_module {
	extern GLuint vao;
	extern GLuint vbo;
	extern GLuint vert_shader, frag_shader;
	extern GLuint shader_program;
	extern unsigned int vertex_count, data_width, data_length;
	
	void load_from_file(const std::string& path, GLenum type);	
	void create_program(const std::string& vert_path, const std::string& frag_path);
	void use();

	void init(float* vertices, unsigned int widht, unsigned int length);
	void render();
	void clean();
}
