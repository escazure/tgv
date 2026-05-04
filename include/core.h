#pragma once

#include "gl3w.h"
#include "glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "stb_image.h"

#include "function_loader.h"
#include "shader_program.h"
#include "simple_camera.h"
#include "terrain.h"

#include <cmath>
#include <string>
#include <vector>

extern Camera camera;
extern FunctionLoader function_loader;
extern Terrain* terrain;

extern bool terrain_generated;
extern bool is_wireframe_mode;
extern bool cool_backface;
extern bool render_skybox;
extern bool show_normals;

extern float window_width;
extern float window_height;

GLFWwindow* init();
void run(GLFWwindow* window);
void shutdown(GLFWwindow* window);
