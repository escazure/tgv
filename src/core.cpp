#include "core.h"
#include "helper.h"
#include "render.h"
#include "state.h"

AppState state;

Camera camera(glm::vec3(0.0, 500.0, 0.0), 150.0, 0.07);
Terrain* terrain;

unsigned int heightMapFBO, heightMap;
unsigned int normalMapFBO, normalMap;

GLFWwindow* init(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "tgv", NULL, NULL);

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
	init_fbo(heightMapFBO, heightMap, 2048, 2048, 1, false);
	init_fbo(normalMapFBO, normalMap, 2048, 2048, 4, false);

	glEnable(GL_DEPTH_TEST);

	state.camera = &camera;
	state.terrain = terrain;
	state.window_width = mode->width;
	state.window_height = mode->height;

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

	Shader shader("shaders/final/vertex.glsl", "shaders/final/fragment.glsl");
	Shader skybox_shader("shaders/skybox/skybox_vertex.glsl", "shaders/skybox/skybox_fragment.glsl");
	Shader normal_map_shader("shaders/normalMapping/normal_map_vertex.glsl", "shaders/normalMapping/normal_map_fragment.glsl");

	while(!glfwWindowShouldClose(window)){
		if(state.cull_backface) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);

		if(state.is_wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClearColor(0.2, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;	
		last_frame = current_frame;

		process_input(window, delta_time);

		if(state.generate_terrain){
			std::string path = std::string(PROJECT_FUNCTIONS_DIR) + "/";
			Shader height_map_shader((path + "height_map_vertex.glsl").c_str(), (path + "height_map_fragment.glsl").c_str());
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			auto start_time = std::chrono::high_resolution_clock::now();

			glBindFramebuffer(GL_FRAMEBUFFER, heightMapFBO);
			glViewport(0, 0, state.size, state.size);
			resize_fbo(heightMap, state.size, state.size, 1, false);
			resize_fbo(normalMap, state.size, state.size, 3, false);
			glm::mat4 model(1.0f);

			height_map_shader.use();
			height_map_shader.set_float("uWorldSize", state.size);
			height_map_shader.set_int("uSeed", state.seed);
			render_quad();	

			auto end_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> duration = end_time - start_time;
			state.gen_time = duration.count();
			std::cout << "Finished generating terrain - " << duration.count() << "ms\n";

			glBindFramebuffer(GL_FRAMEBUFFER, normalMapFBO);

			start_time = std::chrono::high_resolution_clock::now();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, heightMap);

			normal_map_shader.use();
			normal_map_shader.set_int("uHeightMap", 0);
			normal_map_shader.set_float("uTerrainSize", state.size);
			render_quad();	

			end_time = std::chrono::high_resolution_clock::now();
			duration = end_time - start_time;
			std::cout << "Finished computing normals - " << duration.count() << "ms\n";

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			state.generate_terrain = false;
			state.terrain_generated = true;
			if(state.is_wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		}

		if(state.terrain_generated){
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			if(state.cull_backface) glEnable(GL_CULL_FACE);

			glViewport(0, 0, int(state.window_width), int(state.window_height));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, heightMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMap);

			shader.use();
			shader.set_int("heightMap", 0);
			shader.set_int("normalMap", 1);
			shader.set_float("halfTerrainLength", float(state.terrain->terrain_length) * 0.5f);
			shader.set_bool("renderTerrainSkirt", state.render_terrain_skirt);
			shader.set_bool("show_normals", state.show_normals);

			glm::mat4 model(1.0f);
			shader.set_mat4("model", model);
			
			glm::mat4 view = state.camera->get_view_mat();
			shader.set_mat4("view", view);

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->view_distance);
			shader.set_mat4("projection", projection);

			shader.set_vec3("lightDir", lightDir);

			state.terrain->draw();
		}

		if(state.render_skybox){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthMask(GL_FALSE); 
			glDepthFunc(GL_LEQUAL);
			glDisable(GL_CULL_FACE);
			draw_skybox(skybox_shader);
			glDepthMask(GL_TRUE); 
			glDepthFunc(GL_LESS);
		}
		
		render_gui(state);

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
}
