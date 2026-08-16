#pragma once
#include "core.h"
#include "state.h"
#include "texture.h"
#include <fstream>
#include <sstream>
#include <vector>

void process_input(GLFWwindow* window, float delta_time);
void mouse_callback_wrapper(GLFWwindow* window, double xpos, double ypos);
void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods);

std::string wrap_user_input(const std::string& function_body, const std::string& function_name);
void build_shader(const std::string& udf, const std::string& udf_name, const std::string& shader_name);

void getMinMaxHeight(Shader& shader, AppState& state, Texture &heightMap, unsigned int width, unsigned int height);
void debugMessageCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char *msg, const void *data);
