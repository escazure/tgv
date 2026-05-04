#include "render.h"
#include "helper.h"

unsigned int skyboxVAO, skyboxVBO, skybox_cubemap;	

float skyboxVert[] = {
	-0.5, -0.5,  0.5,
     0.5, -0.5,  0.5,
     0.5,  0.5,  0.5,
    -0.5, -0.5,  0.5,
     0.5,  0.5,  0.5,
    -0.5,  0.5,  0.5,

    -0.5, -0.5, -0.5,
     0.5,  0.5, -0.5,
     0.5, -0.5, -0.5,
    -0.5, -0.5, -0.5,
    -0.5,  0.5, -0.5,
     0.5,  0.5, -0.5,

    -0.5, -0.5, -0.5,
    -0.5, -0.5,  0.5,
    -0.5,  0.5,  0.5,
    -0.5, -0.5, -0.5,
    -0.5,  0.5,  0.5,
    -0.5,  0.5, -0.5,

     0.5, -0.5, -0.5,
     0.5,  0.5,  0.5,
     0.5, -0.5,  0.5,
     0.5, -0.5, -0.5,
     0.5,  0.5, -0.5,
     0.5,  0.5,  0.5,

    -0.5,  0.5, -0.5,
    -0.5,  0.5,  0.5,
     0.5,  0.5,  0.5,
    -0.5,  0.5, -0.5,
     0.5,  0.5,  0.5,
     0.5,  0.5, -0.5,

    -0.5, -0.5, -0.5,
     0.5, -0.5,  0.5,
    -0.5, -0.5,  0.5,
    -0.5, -0.5, -0.5,
     0.5, -0.5, -0.5,
     0.5, -0.5,  0.5,	
};

void render_gui(){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 30.0f));
	ImGui::SetNextWindowSize(ImVec2(380.0f, 200.0f));
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

	ImGui::Columns(2, nullptr, false);

	ImGui::Text("Size: %d",size);
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	if(ImGui::SliderInt("##size", &exponent, MIN_EXPONENT, MAX_EXPONENT, "")){
		size = 1 << exponent;	
	}
	ImGui::NextColumn();

	ImGui::Text("Step size: %d", step_size);
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	if(ImGui::SliderInt("##step_exponent", &step_exponent, MIN_STEP_EXPONENT, MAX_STEP_EXPONENT, "")){
		step_size = 1 << step_exponent;
	}
	ImGui::NextColumn();
	
	ImGui::Text("Chunk size: %d",chunk_size);
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	if(ImGui::SliderInt("##chunk_exponent", &chunk_exponent, MIN_CHUNK_EXPONENT, MAX_CHUNK_EXPONENT, "")){
		chunk_size = 1 << chunk_exponent;	
	}
	ImGui::NextColumn();

	ImGui::Text("Function: ");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##fun_buf", fun_buf, IM_ARRAYSIZE(fun_buf));
	ImGui::NextColumn();

	ImGui::Text("Function name: ");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##fun_name", fun_name, IM_ARRAYSIZE(fun_name));
	ImGui::NextColumn();

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

	ImGui::Columns(2, nullptr, false);

	ImGui::Text("Triangle count:");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Text("%d", terrain_generated ? terrain->triangle_count : 0);
	ImGui::NewLine();
	ImGui::NextColumn();

	ImGui::Text("Max height:");	
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Text("%.2f", terrain_generated ? terrain->max_height : 0.0f);	
	ImGui::NextColumn();

	ImGui::Text("Min height:");	
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Text("%.2f", terrain_generated ? terrain->min_height : 0.0f);	
	ImGui::NextColumn();

	ImGui::Text("Height range:");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Text("%.2f", terrain_generated ? terrain->max_height - terrain->min_height : 0.0f);
	ImGui::NewLine();
	ImGui::NextColumn();

	ImGui::Text("Generation time (ms):");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Text("%.2f", terrain_generated ? terrain->gen_time : 0.0f);
	ImGui::NewLine();
	ImGui::NextColumn();

	ImGui::Text("FPS:");
	ImGui::NextColumn();
	ImGui::SetNextItemWidth(-1);
	ImGui::Text("%.1f", ImGui::GetIO().Framerate);
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
		}
		ImGui::End();
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
		}
		ImGui::End();
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
			ImGui::Text("Toggle capture mouse - ESC");
		}
		ImGui::End();
	}

	if(show_render_settings_window){
		ImGui::SetNextWindowSize(ImVec2(600.0f, 300.0f));
		ImGui::SetNextWindowPos(ImVec2(window_width/2 - 300.0f, window_height/2 - 150.0f));
		if(ImGui::Begin("Rendering", &show_render_settings_window)){
			ImGui::Checkbox("Toggle wireframe mode", &is_wireframe_mode);
			ImGui::NewLine();

			ImGui::Checkbox("Toggle backface cooling", &cool_backface);
			ImGui::NewLine();

			ImGui::Checkbox("Toggle skybox", &render_skybox);
			ImGui::NewLine();

			ImGui::Checkbox("Toggle normals", &show_normals);
			ImGui::NewLine();
		}
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void init_skybox(){
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVert), skyboxVert, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	std::vector<std::string> faces = {
		PROJECT_ASSET_DIR + std::string("/textures/skybox/px.png"),
		PROJECT_ASSET_DIR + std::string("/textures/skybox/nx.png"),
		PROJECT_ASSET_DIR + std::string("/textures/skybox/py.png"),
		PROJECT_ASSET_DIR + std::string("/textures/skybox/ny.png"),
		PROJECT_ASSET_DIR + std::string("/textures/skybox/pz.png"),
		PROJECT_ASSET_DIR + std::string("/textures/skybox/nz.png")
	};
	skybox_cubemap = loadCubeMap(faces);
}

void draw_skybox(Shader shader){
	shader.use();

	glm::mat4 view = glm::mat4(glm::mat3(camera.get_view_mat()));
	shader.set_mat4("view", view);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), window_width/window_height, 0.1f, camera.view_distance);
	shader.set_mat4("projection", projection);

	shader.set_int("skybox", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);

	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
