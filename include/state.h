#pragma once
#include <string>

#define MIN_CAMERA_SPEED 50.0f
#define MAX_CAMERA_SPEED 200.0f
#define MIN_CAMERA_VIEW_DISTANCE 1000.0f
#define MAX_CAMERA_VIEW_DISTANCE 10000.0f

#define SHADOW_WIDTH 4096 
#define SHADOW_HEIGHT 4096 

class FunctionLoader;
class Terrain;
struct Camera;

struct AppState{
    int size = 1024;
    int chunk_size = 128;
	int step_size = 1;

    char fun_buf[512] = "example(x,z)";
    char fun_name[128] = "example_terrain";

    bool show_noise_window = false;
    bool show_camera_settings_window = false;
    bool show_keybinds_window = false;

    float window_width = 1920.0f;
    float window_height = 1080.0f;

    FunctionLoader* function_loader = nullptr;
    Terrain* terrain = nullptr;
    Camera* camera = nullptr;

    bool terrain_generated = false;
    bool calculate_lighting = false;
    bool is_wireframe_mode = false;
    bool cull_backface = true;
    bool render_skybox = true;
    bool show_normals = false;
    bool show_light_frustum = false;
    bool show_light_marker = false;
};
