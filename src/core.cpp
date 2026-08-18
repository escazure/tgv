#include "core.h"

AppState state;

Camera camera(glm::vec3(0.0, 500.0, 0.0), 150.0, 0.07);
Terrain* terrain;

Texture heightMap;
Texture normalMap;
Texture shadowMap;

Sampler linearClamp;

unsigned int shadowMapFBO;

GLFWwindow* init(){
	if(state.logging) std::cout << "Logging enabled\n";
	if(state.debug_mode) std::cout << "OpenGL debug callback enabled\n";
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

	linearClamp.create(GL_LINEAR, GL_LINEAR);

	heightMap.create(GL_TEXTURE_2D, 1, GL_R32F, 1024, 1024);
	normalMap.create(GL_TEXTURE_2D, 1, GL_RGBA32F, 1024, 1024);
	shadowMap.create(GL_TEXTURE_2D, 1, GL_R32F, 1024, 1024);

	glCreateFramebuffers(1, &shadowMapFBO);
	shadowMap.attach(shadowMapFBO, GL_COLOR_ATTACHMENT0);

	state.camera = &camera;
	state.terrain = terrain;
	state.window_width = mode->width;
	state.window_height = mode->height;

	if(state.logging){
		std::cout << "Launched with:\n";
		std::cout << " Vendor - " << glGetString(GL_VENDOR) << "\n";
		std::cout << " Renderer - " << glGetString(GL_RENDERER) << "\n";
		std::cout << " Version - " << glGetString(GL_VERSION) << "\n";
		std::cout << " GLSL - " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";
	}

	if(state.debug_mode){
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(debugMessageCallback, NULL);
	}

	glEnable(GL_DEPTH_TEST);

	return window;
}

void run(GLFWwindow* window){
	float delta_time = 0.0;
	float last_frame = 0.0;
	float current_frame = 0.0;

	const glm::vec3 lightDir = glm::normalize(glm::vec3(-2.0f, -1.0f, 0.0f));

	Shader shader("final/vertex.glsl", "final/fragment.glsl");
	Shader skybox_shader("skybox/skybox_vertex.glsl", "skybox/skybox_fragment.glsl");
	Shader normal_map_shader("normalMapping/normal_map.comp");
	Shader shadow_map_shader("shadowMapping/shadow_map_vertex.glsl", "shadowMapping/shadow_map_fragment.glsl");
	Shader min_max_compute_shader("minMaxComp/min_max.comp");

	if(state.logging){
		std::cout << "Shaders id:\n";
		std::cout << " Final - " << shader.id << "\n";
		std::cout << " Skybox - " << skybox_shader.id << "\n";
		std::cout << " Normal Map - " << normal_map_shader.id << "\n";
		std::cout << " Shadow Map - " << shadow_map_shader.id << "\n";
		std::cout << " Min Max Compute - " << min_max_compute_shader.id << "\n";
	}

	std::array<Timer, TIMER_COUNT> timers = {
		Timer("Height Map"),
		Timer("Min Max"),
		Timer("Normal Map"),
		Timer("Shadow Map")
	};

	unsigned int barrier = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT;

	const float skyClearColor[4] = {0.2f, 0.6f, 0.8f, 1.0f};
	const float shadowClearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	const float depthClearColor = 1.0f;

	while(!glfwWindowShouldClose(window)){
		if(state.cull_backface) glEnable(GL_CULL_FACE);
		else glDisable(GL_CULL_FACE);

		if(state.is_wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if(state.is_vsync) glfwSwapInterval(1);
		else glfwSwapInterval(0);

		glClearNamedFramebufferfv(0, GL_COLOR, 0, skyClearColor);
		glClearNamedFramebufferfv(0, GL_DEPTH, 0, &depthClearColor);

		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;	
		last_frame = current_frame;

		process_input(window, delta_time);

		if(state.generate_terrain){
			state.gen_time = 0.0f;
			Shader height_map_shader("heightMap/height_map.comp");

			// ----- Height map generation ----- //
			heightMap.resize(1, state.size, state.size);
			heightMap.bindAsImage(0);
			linearClamp.bind(0);

			height_map_shader.use();
			height_map_shader.set_float("uWorldSize", state.size);
			height_map_shader.set_int("uSeed", state.seed);

			timers[HEIGHT_MAP_ID].begin();
			glDispatchCompute(state.size / 16, state.size / 16, 1);		
			timers[HEIGHT_MAP_ID].end();

			glMemoryBarrier(barrier);
			// ----- Compute min/max height ----- //
			timers[MIN_MAX_ID].begin();
			getMinMaxHeight(min_max_compute_shader, state, heightMap, state.size, state.size);
			timers[MIN_MAX_ID].end();

			// ----- Normal map generation ----- //
			normalMap.resize(1, state.size, state.size);
			normalMap.bindAsImage(1);
			linearClamp.bind(1);

			heightMap.bind(0);
			linearClamp.bind(0);

			normal_map_shader.use();
			normal_map_shader.set_float("uTerrainSize", state.size);

			timers[NORMAL_MAP_ID].begin();
			glDispatchCompute(state.size / 16, state.size / 16, 1);		
			timers[NORMAL_MAP_ID].end();

			glMemoryBarrier(barrier);
			// ----- Shadow map generation ----- //
			shadowMap.resize(1, state.size, state.size);
			shadowMap.attach(shadowMapFBO, GL_COLOR_ATTACHMENT0);

			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
			glViewport(0, 0, state.size, state.size);

			glClearNamedFramebufferfv(shadowMapFBO, GL_COLOR, 0, shadowClearColor);

			heightMap.bind(0);
			normalMap.bind(1);

			linearClamp.bind(0);
			linearClamp.bind(1);

			shadow_map_shader.use();
			shadow_map_shader.set_float("uTerrainSize", state.size);
			shadow_map_shader.set_float("uMaxHeight", state.max_height);
			shadow_map_shader.set_vec3("uLightDir", lightDir);

			timers[SHADOW_MAP_ID].begin();
			render_quad();	
			timers[SHADOW_MAP_ID].end();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			state.generate_terrain = false;
			state.terrain_generated = true;

			if(state.is_wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		if(state.terrain_generated){
			if(state.cull_backface) glEnable(GL_CULL_FACE);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, int(state.window_width), int(state.window_height));

			heightMap.bind(0);
			normalMap.bind(1);
			shadowMap.bind(2);

			linearClamp.bind(0);
			linearClamp.bind(1);
			linearClamp.bind(2);

			shader.use();
			shader.set_float("uTerrainSize", state.size);
			shader.set_float("uChunkSize", state.chunk_size);
			shader.set_float("uMinHeight", state.min_height);
			shader.set_float("uMaxHeight", state.max_height);
			shader.set_int("uTextureMethod", state.texture_method);
			shader.set_bool("uRenderTerrainSkirt", state.render_terrain_skirt);
			shader.set_bool("uShowNormals", state.show_normals);
			shader.set_bool("uCalculateLighting", state.calculate_lighting);
			shader.set_vec3("uLightDir", lightDir);

			glm::mat4 model(1.0f);
			shader.set_mat4("uModel", model);
			
			glm::mat4 view = state.camera->getViewMat();
			shader.set_mat4("uView", view);

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->_viewDistance);
			shader.set_mat4("uProjection", projection);

			state.terrain->draw();
		}

		if(state.render_skybox){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDepthMask(GL_FALSE); 
			glDepthFunc(GL_LEQUAL);
			glDisable(GL_CULL_FACE);
			render_skybox(skybox_shader, lightDir);
			glDepthMask(GL_TRUE); 
			glDepthFunc(GL_LESS);
		}
		
		if(state.show_ui){
			render_gui(state);
		}

		for(std::size_t i = 0; i < timers.size(); i++){
			if(timers[i].isAvailable()){
				double time = timers[i].getMilli();
				if(state.logging)
					std::cout << "Timer [" << timers[i]._name << "] recorded time - " << time << "ms\n";
				state.gen_time += time;
			}
		}

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
