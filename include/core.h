#pragma once

#include "gl3w.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "function_loader.h"
#include "shader_program.h"
#include "simple_renderer.h"
#include "simple_generator.h"
#include "simple_camera.h"

#include <cmath>

GLFWwindow* init(float* vertices, unsigned int size, unsigned int* indices, unsigned int isize);
void run(GLFWwindow* window, float* vertices, unsigned int size);
void shutdown(GLFWwindow* window);
