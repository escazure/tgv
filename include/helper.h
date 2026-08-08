#pragma once

#include "core.h"
#include <fstream>
#include <sstream>

#define SUCCESS 0
#define FAILED_TO_OPEN_FILE 1
#define FAILED_TO_WRITE_FILE 2

void process_input(GLFWwindow* window, float delta_time);
void mouse_callback_wrapper(GLFWwindow* window, double xpos, double ypos);
void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int load_cube_map(std::vector<std::string>& faces);

std::string wrap_user_input(const std::string& function_body, const std::string& function_name);
unsigned int build_shader(const std::string& udf, const std::string& udf_name, const std::string& shader_name);

void getMinMaxHeight(Shader& shader, AppState& state, unsigned int heightMap, unsigned int width, unsigned int height);
