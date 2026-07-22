#pragma once

#include "core.h"

void process_input(GLFWwindow* window, float delta_time);
void mouse_callback_wrapper(GLFWwindow* window, double xpos, double ypos);
void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadCubeMap(std::vector<std::string>& faces);
std::vector<glm::vec3> GetLightFrustumCornersWorldSpace(const glm::mat4& lightSpaceMatrix);
void calculateTightLightProjection(float maxH, float minH, float halfLen, glm::mat4& lightProjection, glm::mat4& lightView, glm::vec3 lightDir);
