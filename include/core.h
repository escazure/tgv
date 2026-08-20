#pragma once

#include "gl3w.h"
#include "glfw/glfw3.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"

#include "shader_program.h"
#include "simple_camera.h"
#include "helper.h"
#include "render.h"
#include "terrain.h"
#include "state.h"
#include "texture.h"
#include "sampler.h"
#include "gpu_timer.h"

#include <cmath>
#include <string>
#include <vector>
#include <array>

extern AppState state;

GLFWwindow* init();
void run(GLFWwindow* window);
void shutdown(GLFWwindow* window);

enum TimerID {
    HEIGHT_MAP_ID,
    MIN_MAX_ID,
    NORMAL_MAP_ID,
    SHADOW_MAP_ID,
    TIMER_COUNT
};
