#include "core.h"
#include "logger.h"

AppState state;

Camera camera(glm::vec3(0.0, 500.0, 0.0), 150.0, 0.07);
Terrain* terrain;

Texture heightMap;
Texture normalMap;
Texture shadowMap;
Texture reflectionMap, reflectionDepth;

GLuint shadowMapFBO, reflectionFBO;

GLFWwindow* init(){
	if(state.logging) log(MESSAGE_INFO, "[CORE] Logging enabled");
	if(state.debug_mode) log(MESSAGE_INFO, "[CORE] OpenGL debug callback enabled");
	
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

	if(gl3wInit() != 0) log(MESSAGE_ERROR, "[GL3W] Failed to init gl3w");

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

	heightMap.create(GL_TEXTURE_2D, 1, GL_R32F, 1024, 1024);
	heightMap.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	heightMap.setFilter(GL_LINEAR, GL_LINEAR);

	normalMap.create(GL_TEXTURE_2D, 1, GL_RGBA32F, 1024, 1024);
	normalMap.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	normalMap.setFilter(GL_LINEAR, GL_LINEAR);

	shadowMap.create(GL_TEXTURE_2D, 1, GL_R32F, 1024, 1024);
	shadowMap.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	shadowMap.setFilter(GL_LINEAR, GL_LINEAR);

	reflectionMap.create(GL_TEXTURE_2D, 1, GL_RGBA8, mode->width, mode->height);
	reflectionMap.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	reflectionMap.setFilter(GL_LINEAR, GL_LINEAR);

	reflectionDepth.create(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, mode->width, mode->height);
	reflectionMap.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	reflectionMap.setFilter(GL_LINEAR, GL_LINEAR);

	state.heightMap = &heightMap;

	glCreateFramebuffers(1, &shadowMapFBO);
	shadowMap.attach(shadowMapFBO, GL_COLOR_ATTACHMENT0);

	glCreateFramebuffers(1, &reflectionFBO);
	reflectionMap.attach(reflectionFBO, GL_COLOR_ATTACHMENT0);
	reflectionDepth.attach(reflectionFBO, GL_DEPTH_ATTACHMENT);

	state.camera = &camera;
	state.terrain = terrain;
	state.window_width = mode->width;
	state.window_height = mode->height;

	if(state.logging){
		log(MESSAGE_INFO, std::format("[CORE] Vendor - {}", getGLString(GL_VENDOR)));
		log(MESSAGE_INFO, std::format("[CORE] Renderer - {}", getGLString(GL_RENDERER)));
		log(MESSAGE_INFO, std::format("[CORE] OpenGL Version - {}", getGLString(GL_VERSION)));
		log(MESSAGE_INFO, std::format("[CORE] GLSL Version - {}", getGLString(GL_SHADING_LANGUAGE_VERSION)));
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
	Shader water_plane_shader("waterPlane/vertex.glsl", "waterPlane/fragment.glsl");
	Shader min_max_compute_shader("minMaxComp/min_max.comp");

	if(state.logging){
		log(MESSAGE_INFO, std::format("[CORE] Final shader id - {}", shader.id));
		log(MESSAGE_INFO, std::format("[CORE] Skybox shader id - {}", skybox_shader.id));
		log(MESSAGE_INFO, std::format("[CORE] Normal map shader id - {}", normal_map_shader.id));
		log(MESSAGE_INFO, std::format("[CORE] Shadow map shader id - {}", shadow_map_shader.id));
		log(MESSAGE_INFO, std::format("[CORE] Min Max Compute shader id - {}", min_max_compute_shader.id));
		log(MESSAGE_INFO, std::format("[CORE] Water plane shader id - {}", water_plane_shader.id));
	}

	std::array<Timer, TIMER_COUNT> timers = {
		Timer("Height Map"),
		Timer("Min Max"),
		Timer("Normal Map"),
		Timer("Shadow Map"),
	};

	unsigned int barrier = GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT;

	const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
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

		glm::vec4 clipPlaneBelow = glm::vec4(0.0f, 1.0f, 0.0f, -state.water_plane_level);

		if(state.generate_terrain){
			state.gen_time = 0.0f;
			Shader height_map_shader("heightMap/height_map.comp");

			// ----- Height map generation ----- //
			if(!state.height_map_imported){
				if(state.logging) log(MESSAGE_INFO, "[CORE RENDER LOOP] Generating heightmap...");
				heightMap.resize(1, state.size, state.size);
				heightMap.bindAsImage(0);

				height_map_shader.use();
				height_map_shader.set_float("uWorldSize", state.size);
				height_map_shader.set_int("uSeed", state.seed);

				timers[HEIGHT_MAP_ID].begin();
				glDispatchCompute(state.size / 16, state.size / 16, 1);		
				timers[HEIGHT_MAP_ID].end();

				glMemoryBarrier(barrier);
				if(state.logging) log(MESSAGE_SUCCESS, "[CORE RENDER LOOP] Done");
			}
			else{
				if(state.logging) log(MESSAGE_INFO, "[CORE RENDER LOOP] Skipped heightmap generation, was imported");
			}

			// ----- Compute min/max height ----- //
			if(!state.height_map_min_max_imported){
				if(state.logging) log(MESSAGE_INFO, "[CORE RENDER LOOP] Calculating min/max heights...");
				timers[MIN_MAX_ID].begin();
				getMinMaxHeight(min_max_compute_shader, state, heightMap, state.size, state.size);
				timers[MIN_MAX_ID].end();
				if(state.logging) log(MESSAGE_SUCCESS, "[CORE RENDER LOOP] Done");
			}
			else{
				if(state.logging) log(MESSAGE_INFO, "[CORE RENDER LOOP] Skipped min/max calculations, were imported");
			}

			// ----- Normalmap generation ----- //
			if(state.logging) log(MESSAGE_INFO, "[CORE RENDER LOOP] Generating normalmap...");
			normalMap.resize(1, state.size, state.size);
			normalMap.bindAsImage(1);

			heightMap.bind(0);

			normal_map_shader.use();
			normal_map_shader.set_float("uTerrainSize", state.size);

			timers[NORMAL_MAP_ID].begin();
			glDispatchCompute(state.size / 16, state.size / 16, 1);		
			timers[NORMAL_MAP_ID].end();

			glMemoryBarrier(barrier);
			if(state.logging) log(MESSAGE_SUCCESS, "[CORE RENDER LOOP] Done");

			// ----- Shadowmap generation ----- //
			if(state.logging) log(MESSAGE_INFO, "[CORE RENDER LOOP] Generating shadowmap...");
			shadowMap.resize(1, state.size, state.size);
			shadowMap.attach(shadowMapFBO, GL_COLOR_ATTACHMENT0);

			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
			glViewport(0, 0, state.size, state.size);

			glClearNamedFramebufferfv(shadowMapFBO, GL_COLOR, 0, shadowClearColor);

			heightMap.bind(0);
			normalMap.bind(1);

			shadow_map_shader.use();
			shadow_map_shader.set_float("uTerrainSize", state.size);
			shadow_map_shader.set_float("uMaxHeight", state.max_height);
			shadow_map_shader.set_vec3("uLightDir", lightDir);

			timers[SHADOW_MAP_ID].begin();
			render_quad();	
			timers[SHADOW_MAP_ID].end();

			if(state.logging) log(MESSAGE_SUCCESS, "[CORE RENDER LOOP] Done");

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			state.generate_terrain = false;
			state.terrain_generated = true;

			if(state.is_wireframe_mode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		if(state.terrain_generated && state.render_water_plane){
			glBindFramebuffer(GL_FRAMEBUFFER, reflectionFBO);
			glViewport(0, 0, int(state.window_width), int(state.window_height));

			glClearNamedFramebufferfv(reflectionFBO, GL_COLOR, 0, clearColor);
			glClearNamedFramebufferfv(reflectionFBO, GL_DEPTH, 0, &depthClearColor);
			
			heightMap.bind(0);
			normalMap.bind(1);
			shadowMap.bind(2);

			if(state.cull_backface){
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
			}

			glm::mat4 view = state.camera->getViewMat();
			glm::mat4 reflection = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, state.water_plane_level, 0.0f));
			reflection = glm::scale(reflection, glm::vec3(1.0f, -1.0f, 1.0f));
			reflection = glm::translate(reflection, glm::vec3(0.0f, -state.water_plane_level, 0.0f));
			glm::mat4 reflectedView = view * reflection;

			glEnable(GL_CLIP_DISTANCE0);
			render_terrain(shader, lightDir, reflectedView, clipPlaneBelow);
			glDisable(GL_CLIP_DISTANCE0);

			if(state.cull_backface) glCullFace(GL_BACK);
		}

		if(state.terrain_generated){
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, int(state.window_width), int(state.window_height));

			heightMap.bind(0);
			normalMap.bind(1);
			shadowMap.bind(2);

			glm::mat4 view = state.camera->getViewMat();

			if(state.clip_terrain_below_water) 
				glEnable(GL_CLIP_DISTANCE0);

			render_terrain(shader, lightDir, view, state.clip_terrain_below_water ? clipPlaneBelow : glm::vec4(0.0f));
			glDisable(GL_CLIP_DISTANCE0);
		}

		if(state.terrain_generated && state.render_water_plane){
			reflectionMap.bind(0);

			render_water_plane(state.size, state.water_plane_level, water_plane_shader, lightDir);
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
				if(state.logging) log(MESSAGE_INFO, std::format("[CORE RENDER LOOP TIMER] {} recorded time - {} ms", timers[i]._name, time));
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
