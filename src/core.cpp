#include "core.h"
#include "helper.h"
#include "render.h"
#include "state.h"

AppState state;

Camera camera(glm::vec3(0.0, 500.0, 0.0), 150.0, 0.07);
Terrain* terrain;

unsigned int heightMapFBO, heightMap;
unsigned int normalMapFBO, normalMap;
unsigned int shadowMapFBO, shadowMap;

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
	init_fbo(heightMapFBO, heightMap, 2048, 2048, 1, 1, true);
	init_fbo(normalMapFBO, normalMap, 2048, 2048, 4, 1, true);
	init_fbo(shadowMapFBO, shadowMap, 2048, 2048, 1, 1, true);

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

	const glm::vec3 lightDir = glm::normalize(glm::vec3(-2.0f, -1.0f, 0.0f));

	Shader shader("shaders/final/vertex.glsl", "shaders/final/fragment.glsl");
	Shader skybox_shader("shaders/skybox/skybox_vertex.glsl", "shaders/skybox/skybox_fragment.glsl");
	Shader normal_map_shader("shaders/normalMapping/normal_map_vertex.glsl", "shaders/normalMapping/normal_map_fragment.glsl");
	Shader shadow_map_shader("shaders/shadowMapping/shadow_map_vertex.glsl", "shaders/shadowMapping/shadow_map_fragment.glsl");
	Shader min_max_compute_shader("shaders/minMaxComp/min_max.comp");

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

			// ----- Height map generation ----- //
			resize_fbo_attachment(heightMapFBO, heightMap, state.size, state.size, 1, 1, true);
			glBindFramebuffer(GL_FRAMEBUFFER, heightMapFBO);
			glViewport(0, 0, state.size, state.size);
    		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    		glClear(GL_COLOR_BUFFER_BIT);

			auto start_time = std::chrono::high_resolution_clock::now();

			height_map_shader.use();
			height_map_shader.set_float("uWorldSize", state.size);
			height_map_shader.set_int("uSeed", state.seed);

			render_quad();	
			glFinish();

			auto end_time = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float, std::milli> duration = end_time - start_time;
			state.gen_time = duration.count();
			std::cout << "Finished generating terrain - " << duration.count() << "ms\n";

			// ----- Compute min/max height ----- //
			start_time = std::chrono::high_resolution_clock::now();

			getMinMaxHeight(min_max_compute_shader, state, heightMap, state.size, state.size);
			glFinish();

			end_time = std::chrono::high_resolution_clock::now();
			duration = end_time - start_time;
			state.gen_time += duration.count();
			std::cout << "Finished computing min/max height - " << duration.count() << "ms\n";

			// ----- Normal map generation ----- //
			resize_fbo_attachment(normalMapFBO, normalMap, state.size, state.size, 4, 1, true);
			glBindFramebuffer(GL_FRAMEBUFFER, normalMapFBO);
			glViewport(0, 0, state.size, state.size);
    		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    		glClear(GL_COLOR_BUFFER_BIT);

			start_time = std::chrono::high_resolution_clock::now();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, heightMap);

			normal_map_shader.use();
			normal_map_shader.set_int("uHeightMap", 0);
			normal_map_shader.set_float("uTerrainSize", state.size);

			render_quad();	
			glFinish();

			end_time = std::chrono::high_resolution_clock::now();
			duration = end_time - start_time;
			state.gen_time += duration.count();
			std::cout << "Finished computing normals - " << duration.count() << "ms\n";

			// ----- Shadow map generation ----- //
			resize_fbo_attachment(shadowMapFBO, shadowMap, state.size, state.size, 1, 1, true);
			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
			glViewport(0, 0, state.size, state.size);
    		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    		glClear(GL_COLOR_BUFFER_BIT);

			start_time = std::chrono::high_resolution_clock::now();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, heightMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMap);

			shadow_map_shader.use();
			shadow_map_shader.set_int("uHeightMap", 0);
			shadow_map_shader.set_int("uNormalMap", 1);
			shadow_map_shader.set_float("uTerrainSize", state.size);
			shadow_map_shader.set_float("uMaxHeight", state.max_height);
			shadow_map_shader.set_vec3("uLightDir", lightDir);

			render_quad();	
			glFinish();

			end_time = std::chrono::high_resolution_clock::now();
			duration = end_time - start_time;
			state.gen_time += duration.count();
			std::cout << "Finished computing shadows - " << duration.count() << "ms\n";

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
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, shadowMap);

			shader.use();
			shader.set_int("uHeightMap", 0);
			shader.set_int("uNormalMap", 1);
			shader.set_int("uShadowMap", 2);
			shader.set_float("uTerrainSize", state.size);
			shader.set_bool("uRenderTerrainSkirt", state.render_terrain_skirt);
			shader.set_bool("uShowNormals", state.show_normals);
			shader.set_bool("uCalculateLighting", state.calculate_lighting);
			shader.set_vec3("uLightDir", lightDir);

			glm::mat4 model(1.0f);
			shader.set_mat4("uModel", model);
			
			glm::mat4 view = state.camera->get_view_mat();
			shader.set_mat4("uView", view);

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->view_distance);
			shader.set_mat4("uProjection", projection);

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
