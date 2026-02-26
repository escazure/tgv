#include "core.h"

#define MAX_LENGTH_EXPONENT 11
#define MIN_LENGTH_EXPONENT 5
#define MAX_WIDTH_EXPONENT 11
#define MIN_WIDTH_EXPONENT 5
#define MIN_STEP_SIZE 0.2f
#define MAX_STEP_SIZE 4.0f

Camera camera(glm::vec3(0.0, 50.0, 0.0), 20.0, 0.07);
Terrain* terrain;
FunctionLoader function_loader;

float lastx, lasty, delta_time;
bool first_mouse = true;
bool is_capturing = false;
bool terrain_generated = false;

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
	static int x_exponent = MIN_LENGTH_EXPONENT;
	static int z_exponent = MIN_WIDTH_EXPONENT;
	static int x_size = 1 << x_exponent;
	static int z_size = 1 << z_exponent; 
	static float step_size = 1.0f;
	static char fun_buf[256];
	static char fun_name[128];
	static bool show_noise_window = false;

	if(ImGui::SliderInt("##xsize", &x_exponent, MIN_LENGTH_EXPONENT, MAX_LENGTH_EXPONENT, "")){
		x_size = 1 << x_exponent;	
	}
	ImGui::SameLine();
	ImGui::Text("X size: %d",x_size);

	if(ImGui::SliderInt("##zsize", &z_exponent, MIN_WIDTH_EXPONENT, MAX_WIDTH_EXPONENT, "")){
		z_size = 1 << z_exponent;	
	}
	ImGui::SameLine();
	ImGui::Text("Z size: %d",z_size);

	ImGui::SliderFloat("##step_size", &step_size, MIN_STEP_SIZE, MAX_STEP_SIZE, "");
	ImGui::SameLine();
	ImGui::Text("Step size: %.1f", step_size);

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
			
		terrain = new Terrain(x_size, z_size, step_size);
		terrain->generate(fun);

		terrain_generated = true;
	}

	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(1920.0f - 350.0f, 30.0f));
	ImGui::SetNextWindowSize(ImVec2(350.0f, 200.0f));
	ImGui::Begin("Statistics");

	ImGui::Text("Max height: %.2f", terrain_generated ? terrain->max : 0.0f);	
	ImGui::Text("Min height: %.2f", terrain_generated ? terrain->min : 0.0f);	
	ImGui::Text("Height range: %.2f", terrain_generated ? terrain->max - terrain->min : 0.0f);
	ImGui::NewLine();
	ImGui::Text("Percent below sea level: %.2f", terrain_generated ? terrain->below_sea : 0.0f);
	ImGui::NewLine();
	ImGui::Text("Generation time (ms): %.2f", terrain_generated ? terrain->gen_time : 0.0f);
	
	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(1920.0f, 40.0f));
	ImGui::BeginMainMenuBar();

	if(ImGui::BeginMenu("File")){
		ImGui::MenuItem("Save");
		ImGui::EndMenu();
	}

	if(ImGui::BeginMenu("Docs")){
		if(ImGui::MenuItem("Noise and utility functions")){
			show_noise_window = true;
		}
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();

	if(show_noise_window){
		ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
		ImGui::SetNextWindowPos(ImVec2(1920.0f/2 - 300.0f, 1080.0f/2 - 150.0f));
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

	glfwMakeContextCurrent(window);

	lastx = 1980/2;
	lasty = 1080/2;

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

	Shader shader("vertex.glsl", "fragment.glsl");
	shader.use();
	
	while(!glfwWindowShouldClose(window)){
		glClearColor(0.2, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;	
		last_frame = current_frame;

		// -----------------------------------------------------------------
		//  Process input, send data to shader and render only if terrain already generated 
		// -----------------------------------------------------------------
		
		if(terrain_generated){
			process_input(window);

			glm::mat4 model(1.0f);
			shader.set_mat4("model", model);
			
			glm::mat4 view(1.0f);
			view = camera.get_view_mat();
			shader.set_mat4("view", view);

			glm::mat4 projection(1.0f);
			projection = glm::perspective(glm::radians(45.0f), (float)1920/(float)1080, 0.1f, 2000.0f);
			shader.set_mat4("projection", projection);

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
