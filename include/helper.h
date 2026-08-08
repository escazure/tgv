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
unsigned int loadCubeMap(std::vector<std::string>& faces);
std::vector<glm::vec3> GetLightFrustumCornersWorldSpace(const glm::mat4& lightSpaceMatrix);
void calculateTightLightProjection(float maxH, float minH, float halfLen, glm::mat4& lightProjection, glm::mat4& lightView, glm::vec3 lightDir);

std::string wrap_user_input(const std::string& function_body, const std::string& function_name);
unsigned int build_shader(const std::string& udf, const std::string& udf_name, const std::string& shader_name);
