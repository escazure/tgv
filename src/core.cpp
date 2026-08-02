#include "core.h"
#include "helper.h"
#include "render.h"

Camera camera(glm::vec3(0.0, 500.0, 0.0), 150.0, 0.07);
Terrain* terrain;
FunctionLoader function_loader;

bool terrain_generated = false;
bool is_wireframe_mode = false;
bool cull_backface = true;
bool render_skybox = true;
bool show_normals = false;
bool calculate_lighting = false;
bool show_light_frustum = false;
bool show_light_marker = false;
float window_width, window_height;
float bias;
unsigned int depthMapFBO, depthMap;

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
	init_fbo(depthMapFBO, depthMap);
	init_light_frustum();

	glEnable(GL_DEPTH_TEST);

	return window;
}

void run(GLFWwindow* window){
	float delta_time = 0.0;
	float last_frame = 0.0;
	float current_frame = 0.0;

	const glm::vec3 lightDir = glm::normalize(glm::vec3(-3.0f, -1.0f, 0.0f));
	glm::vec3 lightPos = -lightDir * 512.0f;

	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-512.0f, 512.0f, -512.0f, 512.0f, 1.0f, 512.0f);
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	Shader skybox_shader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");
	Shader depth_shader("shaders/depth_vertex.glsl", "shaders/depth_fragment.glsl");
	Shader debug_line_shader("shaders/debugLineVertex.glsl", "shaders/debugLineFragment.glsl");

	while(!glfwWindowShouldClose(window)){
		if(cull_backface) glEnable(GL_CULL_FACE);
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
			calculateTightLightProjection(float(terrain->max_height), float(terrain->min_height), float(terrain->terrain_length >> 1), lightProjection, lightView, lightDir);

			lightSpaceMatrix = lightProjection * lightView;

			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glDisable(GL_CULL_FACE);
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(2.0f, 4.0f);

			glm::mat4 model(1.0f);

			depth_shader.use();
			depth_shader.set_mat4("lightSpaceMatrix", lightSpaceMatrix);
			depth_shader.set_mat4("model", model);

			terrain->draw();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_POLYGON_OFFSET_FILL);
			if(cull_backface) glEnable(GL_CULL_FACE);

			glViewport(0, 0, int(window_width), int(window_height));
			glBindTexture(GL_TEXTURE_2D, depthMap);

			shader.use();
			shader.set_mat4("model", model);
			
			glm::mat4 view = camera.get_view_mat();
			shader.set_mat4("view", view);

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), window_width/window_height, 0.1f, camera.view_distance);
			shader.set_mat4("projection", projection);

			shader.set_mat4("lightSpaceMatrix", lightSpaceMatrix);

			shader.set_float("max_bias", 0.0001);
			shader.set_float("min_bias", 0.00001);
			shader.set_bool("show_normals", show_normals);
			shader.set_bool("calculate_lighting", calculate_lighting);
			shader.set_vec3("lightDir", lightDir);

			terrain->draw();
		}

		if(render_skybox){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthMask(GL_FALSE); 
			glDepthFunc(GL_LEQUAL);
			glDisable(GL_CULL_FACE);
			draw_skybox(skybox_shader);
			glDepthMask(GL_TRUE); 
			glDepthFunc(GL_LESS);
		}

		if(show_light_frustum) 
			DrawLightFrustum(lightSpaceMatrix, debug_line_shader);
			
		if(show_light_marker)
			DrawLightMarker(lightPos, lightDir, debug_line_shader);
		
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
