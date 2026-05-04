#include "core.h"
#include "helper.h"
#include "render.h"

Camera camera(glm::vec3(0.0, 50.0, 0.0), 20.0, 0.07);
Terrain* terrain;
FunctionLoader function_loader;

bool terrain_generated = false;
bool is_wireframe_mode = false;
bool cool_backface = true;
bool render_skybox = true;
float window_width, window_height;

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

	glfwSetCursorPosCallback(window, mouse_callback_wrapper);
	glfwSetKeyCallback(window, key_callback_wrapper);

	if(gl3wInit() != 0)
		std::cerr << "ERROR: Failed to init gl3w" << std::endl;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	(void)io;
	io.IniFilename = nullptr;
	io.Fonts->Clear();
	std::string font_path = PROJECT_ASSET_DIR + std::string("/fonts/URWGothic-Demi.ttf");
	io.Fonts->AddFontFromFileTTF(font_path.c_str(), 16.0f);
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	init_skybox();

	glEnable(GL_DEPTH_TEST);

	return window;
}

void run(GLFWwindow* window){
	float delta_time = 0.0;
	float last_frame = 0.0;
	float current_frame = 0.0;

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	Shader skybox_shader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");

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

		// ----------------------------------------------------------------------------------- //
		//  Process input, send data to shader and render only if terrain is already generated //
		// ----------------------------------------------------------------------------------- //
		
		process_input(window, delta_time);

		if(terrain_generated){
			shader.use();
			glm::mat4 model(1.0f);
			shader.set_mat4("model", model);
			
			glm::mat4 view = camera.get_view_mat();
			shader.set_mat4("view", view);

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), window_width/window_height, 0.1f, camera.view_distance);
			shader.set_mat4("projection", projection);

			shader.set_float("min_y", terrain->min_height);
			shader.set_float("max_y", terrain->max_height);

			terrain->draw();
		}

		if(render_skybox){
			glDepthMask(GL_FALSE); 
			glDepthFunc(GL_LEQUAL);
			glDisable(GL_CULL_FACE);
			draw_skybox(skybox_shader);
			glDepthMask(GL_TRUE); 
			glDepthFunc(GL_LESS);
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
