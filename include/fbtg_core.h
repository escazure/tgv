#pragma once

#include "../external/gl3w.h"
#include "fbtg_callbacks.h"
#include "fbtg_render.h"
#include "fbtg_gen.h"
#include "input.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <dlfcn.h>
#include <cmath>

void parse_config();
GLFWwindow* init_subsystems(float* vertices, u32 width, u32 length);
void run(GLFWwindow* _window);
void shutdown(void** _handle, GLFWwindow* _window);
