#pragma once

#define MIN_CAMERA_SPEED 50.0f
#define MAX_CAMERA_SPEED 500.0f
#define MIN_CAMERA_VIEW_DISTANCE 1000.0f
#define MAX_CAMERA_VIEW_DISTANCE 20000.0f

#define SHADOW_WIDTH 4096 
#define SHADOW_HEIGHT 4096 

class FunctionLoader;
class Terrain;
struct Camera;

struct AppState{
    int size = 1024;
    int chunk_size = 128;
	int step_size = 1;
	int seed = 0;

    char fun_buf[512] = "return example(uv, 0.0003, 2200.0, 2.7, 8);";
    char fun_name[128] = "example_terrain";

    bool show_noise_window = false;
    bool show_camera_settings_window = false;
    bool show_keybinds_window = false;

    float window_width = 1920.0f;
    float window_height = 1080.0f;
	float gen_time = 0.0f;
	float min_height = 0.0f;
	float max_height = 0.0f;

	int texture_method = 0;

    Terrain* terrain = nullptr;
    Camera* camera = nullptr;

	bool debug_mode = false;
	bool logging = false;

    bool terrain_generated = false;
	bool generate_terrain = false;
    bool calculate_lighting = false;
    bool is_wireframe_mode = false;
    bool cull_backface = true;
    bool render_skybox = true;
    bool show_normals = false;
    bool show_light_frustum = false;
    bool show_light_marker = false;
	bool render_terrain_skirt = false;
	bool show_ui = true;
	bool is_vsync = true;
};
