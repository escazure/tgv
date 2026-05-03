#pragma once

#include "core.h"

void process_input(GLFWwindow* window, float delta_time);
void mouse_callback_wrapper(GLFWwindow* window, double xpos, double ypos);
void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods);
