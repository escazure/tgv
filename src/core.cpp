#include "core.h"

#define MAX_EXPONENT 13
#define MIN_EXPONENT 5
#define MIN_STEP_EXPONENT 0
#define MAX_STEP_EXPONENT 3
#define MIN_CHUNK_EXPONENT 5
#define MAX_CHUNK_EXPONENT 8
#define MIN_CAMERA_SPEED 10.0f
#define MAX_CAMERA_SPEED 100.0f
#define MIN_CAMERA_VIEW_DISTANCE 20.0f
#define MAX_CAMERA_VIEW_DISTANCE 10000.0f

Camera camera(glm::vec3(0.0, 50.0, 0.0), 20.0, 0.07);
Terrain* terrain;
FunctionLoader function_loader;

float lastx, lasty, delta_time;
bool first_mouse = true;
bool is_capturing = false;
bool terrain_generated = false;
bool is_wireframe_mode = false;
bool cool_backface = true;
float window_width, window_height;

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	if(!is_capturing){
		first_mouse = true;
		return;
	}
		
	float yawt = camera.yaw, pitcht = camera.pitch;

	if(first_mouse){
		lastx = (float)xpos;
		lasty = (float)ypos;
		first_mouse = false;
	}

	float xoffset = xpos - lastx;
	float yoffset = lasty - ypos;
	lastx = xpos;
	lasty = ypos;

	camera.process_mouse_mov(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		if(ImGui::GetIO().WantCaptureKeyboard) return;
		is_capturing = !is_capturing;

		glfwSetInputMode(window, GLFW_CURSOR, is_capturing ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
}

void process_input(GLFWwindow* window){
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)	
		camera.move_forward(delta_time);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.move_back(delta_time);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.move_left(delta_time);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.move_right(delta_time);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.move_up(delta_time);
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.move_down(delta_time);
}

void render_gui(){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 30.0f));
	ImGui::SetNextWindowSize(ImVec2(350.0f, 200.0f));
	
	ImGui::Begin("Generation");

	// ----- Values below are static since we want to keep them across loop iterations without updating ----- //
	static int exponent = MIN_EXPONENT;
	static int size = 1 << exponent;
	static int step_exponent = MIN_STEP_EXPONENT;
	static int step_size = 1 << step_exponent;
	static int chunk_exponent = MIN_CHUNK_EXPONENT;
	static int chunk_size = 1 << chunk_exponent;
	static char fun_buf[256];
	static char fun_name[128];
	static bool show_noise_window = false;
	static bool show_camera_settings_window = false;
	static bool show_render_settings_window = false;
	static bool show_keybinds_window = false;

	if(ImGui::SliderInt("##size", &exponent, MIN_EXPONENT, MAX_EXPONENT, "")){
		size = 1 << exponent;	
	}
	ImGui::SameLine();
	ImGui::Text("Size: %d",size);

	if(ImGui::SliderInt("##step_exponent", &step_exponent, MIN_STEP_EXPONENT, MAX_STEP_EXPONENT, "")){
		step_size = 1 << step_exponent;
	}
	ImGui::SameLine();
	ImGui::Text("Step size: %d", step_size);
	
	if(ImGui::SliderInt("##chunk_exponent", &chunk_exponent, MIN_CHUNK_EXPONENT, MAX_CHUNK_EXPONENT, "")){
		chunk_size = 1 << chunk_exponent;	
	}
	ImGui::SameLine();
	ImGui::Text("Chunk size: %d",chunk_size);

	ImGui::InputText("Function", fun_buf, IM_ARRAYSIZE(fun_buf));
	ImGui::InputText("Function name", fun_name, IM_ARRAYSIZE(fun_name));
	if(ImGui::Button("Generate")){
		function_loader.function_buffer = std::string(fun_buf);
		function_loader.function_name = std::string(fun_name);
		if(!function_loader.load())
			std::cerr << "ERROR::FUNCTION_LOADER::FAILED_TO_LOAD_FUNCTION" << std::endl;

		float (*fun)(float,float);
	
		if(!function_loader.getFunctionPointer(&fun))
			std::cerr << "ERROR::FUNCTION_LOADER::FAILED_TO_GET_FUNCTION_POINTER" << std::endl;
			
		terrain = new Terrain(size, step_size, chunk_size);
		terrain->generate(fun);

		terrain_generated = true;
	}

	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(window_width - 350.0f, 30.0f));
	ImGui::SetNextWindowSize(ImVec2(350.0f, 230.0f));
	ImGui::Begin("Statistics");

	ImGui::Text("Triangle count: %d", terrain_generated ? terrain->triangle_count : 0);
	ImGui::NewLine();

	ImGui::Text("Max height: %.2f", terrain_generated ? terrain->max_height : 0.0f);	
	ImGui::Text("Min height: %.2f", terrain_generated ? terrain->min_height : 0.0f);	
	ImGui::Text("Height range: %.2f", terrain_generated ? terrain->max_height - terrain->min_height : 0.0f);
	ImGui::NewLine();
	ImGui::Text("Generation time (ms): %.2f", terrain_generated ? terrain->gen_time : 0.0f);
	ImGui::NewLine();
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(window_width, 40.0f));
	ImGui::BeginMainMenuBar();

	if(ImGui::BeginMenu("File")){
		ImGui::MenuItem("Save");
		ImGui::EndMenu();
	}

	if(ImGui::BeginMenu("Settings")){
		if(ImGui::MenuItem("Camera")) show_camera_settings_window = true;
		if(ImGui::MenuItem("Render")) show_render_settings_window = true;
		ImGui::EndMenu();
	}

	if(ImGui::BeginMenu("Docs")){
		if(ImGui::MenuItem("Noise and utility functions")) show_noise_window = true;
		if(ImGui::MenuItem("Keybinds")) show_keybinds_window = true;
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if(show_noise_window){
		ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
		ImGui::SetNextWindowPos(ImVec2(window_width/2 - 300.0f, window_height/2 - 150.0f));
		if(ImGui::Begin("Noise and utility functions", &show_noise_window)){
			ImGui::TextWrapped("* float random(float x, float z, unsigned int seed = 5527265) - returns semi-random values in range (0,1)");
			ImGui::TextWrapped("* float lerp(float a, float b, float t) - interpolates between a and b");
			ImGui::TextWrapped("* float smooth(float t) - returns smooth value");
			ImGui::TextWrapped("* float bell_curve(float x, float z, float radius, float amplitude, int x_offset = 64, int y_offset = 64) - creates a bell curve with given parameters");
			ImGui::TextWrapped("* float value_noise(float x, float z) - creates a value noise map");
			ImGui::TextWrapped("* float fbm(float x, float z, int octaves = 4) - creates a fBm noise map");
			ImGui::TextWrapped("* float example(float x, float z, seed = 1000) - creates an example terrain using multiple stacked fBm calls (high seed values result in blocky terrain, because of rounding error)");
			ImGui::End();
		}
	}

	if(show_camera_settings_window){
		ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
		ImGui::SetNextWindowPos(ImVec2(window_width/2 - 300.0f, window_height/2 - 150.0f));
		if(ImGui::Begin("Camera settings", &show_camera_settings_window)){
			ImGui::Text("Speed: %.1f", camera.speed);
			ImGui::SameLine();
			ImGui::SliderFloat("##camera_speed", &camera.speed, MIN_CAMERA_SPEED, MAX_CAMERA_SPEED, "");
			ImGui::Text("View distance: %.1f", camera.view_distance);
			ImGui::SameLine();
			ImGui::SliderFloat("##camera_view_distance", &camera.view_distance, MIN_CAMERA_VIEW_DISTANCE, MAX_CAMERA_VIEW_DISTANCE, "");
			ImGui::End();
		}
	}

	if(show_keybinds_window){
		ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
		ImGui::SetNextWindowPos(ImVec2(window_width/2 - 300.0f, window_height/2 - 150.0f));
		if(ImGui::Begin("Keybinds", &show_keybinds_window)){
			ImGui::Text("Camera movement left - A");
			ImGui::Text("Camera movement right - D");
			ImGui::Text("Camera movement back - S");
			ImGui::Text("Camera movement forward - W");
			ImGui::Text("Camera movement down - Ctrl");
			ImGui::Text("Camera movement up - Space");
			ImGui::End();
		}
	}

	if(show_render_settings_window){
		ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
		ImGui::SetNextWindowPos(ImVec2(window_width/2 - 300.0f, window_height/2 - 150.0f));
		if(ImGui::Begin("Rendering", &show_render_settings_window)){
			ImGui::Checkbox("Toggle wireframe mode", &is_wireframe_mode);
			ImGui::NewLine();

			ImGui::Checkbox("Toggle backface cooling", &cool_backface);
			ImGui::NewLine();

			ImGui::End();
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

GLFWwindow* init(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "tgv", NULL, NULL);
	window_width = mode->width;
	window_height = mode->height;

	glfwMakeContextCurrent(window);

	lastx = window_width/2;
	lasty = window_height/2;

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	if(gl3wInit() != 0)
		std::cerr << "ERROR: Failed to init gl3w" << std::endl;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	(void)io;
	io.IniFilename = nullptr;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glEnable(GL_DEPTH_TEST);

	return window;
}

void run(GLFWwindow* window){
	delta_time = 0.0;
	float last_frame = 0.0;
	float current_frame = 0.0;

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	shader.use();

	while(!glfwWindowShouldClose(window)){
		if(cool_backface) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);

		if(is_wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClearColor(0.2, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;	
		last_frame = current_frame;

		// -----------------------------------------------------------------
		//  Process input, send data to shader and render only if terrain already generated 
		// -----------------------------------------------------------------
		
		if(is_capturing)
			process_input(window);

		if(terrain_generated){
			glm::mat4 model(1.0f);
			shader.set_mat4("model", model);
			
			glm::mat4 view(1.0f);
			view = camera.get_view_mat();
			shader.set_mat4("view", view);

			glm::mat4 projection(1.0f);
			projection = glm::perspective(glm::radians(45.0f), window_width/window_height, 0.1f, camera.view_distance);
			shader.set_mat4("projection", projection);

			shader.set_float("min_y", terrain->min_height);
			shader.set_float("max_y", terrain->max_height);

			terrain->draw();
		}
		
		render_gui();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void shutdown(GLFWwindow* window){
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);	
	glfwTerminate();
	function_loader.destroy();
}
