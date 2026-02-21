#include "core.h"

#define MAX_TERRAIN_LENGTH 2048;
#define MAX_TERRAIN_WIDTH 2048;

Renderer rend;
Camera camera(glm::vec3(0.0, 50.0, 0.0), 20.0, 0.07);
float lastx, lasty, delta_time;
bool first_mouse = true;
bool capture = false;
bool terrain_generated = false;
int terrain_length = 0, terrain_width = 0;
char fun_buf[512], fun_name[512];

float* vertices;
unsigned int* indices;
unsigned int size, isize;

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
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
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		if(!ImGui::GetIO().WantCaptureKeyboard && capture){
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			std::cout << "capturing\n";
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			std::cout << "not capturing\n";
		}
		capture = !capture;
	}	
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

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	if(gl3wInit() != 0)
		std::cerr << "ERROR: Failed to init gl3w" << std::endl;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glEnable(GL_DEPTH_TEST);

	return window;
}

void runStartUI(GLFWwindow* window){
	while(!glfwWindowShouldClose(window) && !terrain_generated){
		glClearColor(0.2f, 0.6f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize(ImVec2(200.0f, 200.0f));
		
		ImGui::Begin("Generate terrain");
		ImGui::InputInt("X size", &terrain_length);
		ImGui::InputInt("Z size", &terrain_width);
		ImGui::InputText("Function", fun_buf, IM_ARRAYSIZE(fun_buf));
		ImGui::InputText("Function name", fun_name, IM_ARRAYSIZE(fun_name));
		if(ImGui::Button("Generate")){
			if(!write_fun(fun_buf, fun_name))
				std::cerr << "Failed to open/create file" << std::endl;

			if(!compile_fun(fun_name))
				std::cerr << "Failed to compile file" << std::endl;

			float (*fun)(float,float);
			void* handle;
	
			if(!get_fun_pointer(fun_name, &fun, &handle))
				std::cerr << "Failed to get function pointer" << std::endl;
			
			unsigned int vertex_count_x = std::floor(terrain_length/1.0f);
			unsigned int vertex_count_z = std::floor(terrain_width/1.0f);
			
			vertices = generate_vertices(terrain_length, terrain_width, 1.0f, fun);	
			size = vertex_count_x * vertex_count_z * 6;
			indices = generate_indices(terrain_length, terrain_width, 1.0f);
			isize = (vertex_count_x-1)*(vertex_count_z-1)*6;	

			terrain_generated = true;
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	rend.init_renderer(vertices, size, indices, isize, true);
}

void run(GLFWwindow* window){
	delta_time = 0.0;
	float last_frame = 0.0;
	float current_frame = 0.0;

	Shader shader("vertex.glsl", "fragment.glsl");
	shader.use();
	
	glm::mat4 model(1.0f);
	shader.set_mat4("model", model);
	
	float timer = 0.0, prev_timer = 0.0;
	float fps = 0.0;
	while(!glfwWindowShouldClose(window)){
		glClearColor(0.2, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;	
		last_frame = current_frame;
		
		process_input(window);

		if(timer - prev_timer >= 1){
			prev_timer = timer;
			fps = 1/delta_time;
		}

		glm::mat4 view(1.0f);
		view = camera.get_view_mat();
		shader.set_mat4("view", view);

		glm::mat4 projection(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)1920/(float)1080, 0.1f, 1000.0f);
		shader.set_mat4("projection", projection);

		rend.render(size);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize(ImVec2(200.0f, 100.0f));

		ImGui::Begin("Statistics");
		ImGui::Text("FPS: %.2f", fps);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

	delete[] vertices;
	delete[] indices;
}
