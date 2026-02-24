#include "core.h"

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

void render_gui(float fps){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowSize(ImVec2(300.0f, 270.0f));
	
	ImGui::Begin("Generation");

	// ----- Values below are static since we want to keep them across loop iterations without updating ----- //
	static int x_size;
	static int z_size; 
	static float step_size;
	static char fun_buf[512];
	static char fun_name[128];

	ImGui::InputInt("X size", &x_size);
	ImGui::InputInt("Z size", &z_size);
	ImGui::InputFloat("Step size", &step_size);
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

	ImGui::Text("FPS: %.2f", fps);
	ImGui::End();

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
	ImGuiIO& io = ImGui::GetIO(); (void)io;
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
	
	float timer = 0.0, prev_timer = 0.0;
	float fps = 0.0;
	while(!glfwWindowShouldClose(window)){
		glClearColor(0.2, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;	
		last_frame = current_frame;

		if(timer - prev_timer >= 1){
			prev_timer = timer;
			fps = 1/delta_time;
		}

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
			projection = glm::perspective(glm::radians(45.0f), (float)1920/(float)1080, 0.1f, 1000.0f);
			shader.set_mat4("projection", projection);

			terrain->draw();
		}
		
		render_gui(fps);

		glfwSwapBuffers(window);
		glfwPollEvents();
		timer = glfwGetTime();
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
