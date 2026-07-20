#include "core.h"
#include "helper.h"
#include "render.h"

Camera camera(glm::vec3(0.0, 50.0, 0.0), 70.0, 0.07);
Terrain* terrain;
FunctionLoader function_loader;

bool terrain_generated = false;
bool is_wireframe_mode = false;
bool cool_backface = true;
bool render_skybox = true;
bool show_normals = false;
bool calculate_lighting = false;
float window_width, window_height;
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

	glEnable(GL_DEPTH_TEST);

	return window;
}

// Move those 3 somewhere from here // 
std::vector<glm::vec3> GetLightFrustumCornersWorldSpace(const glm::mat4& lightSpaceMatrix) {
    glm::mat4 invLightSpace = glm::inverse(lightSpaceMatrix);
    
    std::vector<glm::vec3> corners;
    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                glm::vec4 pt = invLightSpace * glm::vec4(
                    x * 2.0f - 1.0f,
                    y * 2.0f - 1.0f,
                    z * 2.0f - 1.0f,
                    1.0f
                );
                corners.push_back(glm::vec3(pt) / pt.w);
            }
        }
    }
    return corners;
}

void DrawLightFrustum(GLuint frustumVAO, GLuint frustumVBO, const glm::mat4& lightSpaceMatrix, Shader& debugLineShader) {
    std::vector<glm::vec3> corners = GetLightFrustumCornersWorldSpace(lightSpaceMatrix);

    glBindBuffer(GL_ARRAY_BUFFER, frustumVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, corners.size() * sizeof(glm::vec3), corners.data());

    debugLineShader.use();
	glm::mat4 cameraProjection = glm::perspective(glm::radians(45.0f), window_width/window_height, 0.1f, camera.view_distance);
	glm::mat4 cameraView = camera.get_view_mat();
	glm::mat4 model(1.0f);
    debugLineShader.set_mat4("projection", cameraProjection);
    debugLineShader.set_mat4("view", cameraView);
    debugLineShader.set_mat4("model", model); 
    debugLineShader.set_vec3("color", glm::vec3(1.0f, 1.0f, 0.0f));

    glBindVertexArray(frustumVAO);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void DrawLightMarker(glm::vec3 lightPos, glm::vec3 lightDir, Shader& debugLineShader) {
    glm::vec3 linePoints[2] = {
        lightPos,
        lightPos + (lightDir * 500.0f) 
    };

    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(linePoints), linePoints, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    debugLineShader.use();

	glm::mat4 cameraProjection = glm::perspective(glm::radians(45.0f), window_width/window_height, 0.1f, camera.view_distance);
	glm::mat4 cameraView = camera.get_view_mat();
	glm::mat4 model(1.0f);
    debugLineShader.set_mat4("projection", cameraProjection);
    debugLineShader.set_mat4("view", cameraView);
    debugLineShader.set_mat4("model", model);
    debugLineShader.set_vec3("color", glm::vec3(1.0f, 0.0f, 0.0f)); 

    glLineWidth(3.0f); 
    glDrawArrays(GL_LINES, 0, 2);

    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &lineVAO);
}
// ----------------------------------------------------------------- //

void run(GLFWwindow* window){
	float delta_time = 0.0;
	float last_frame = 0.0;
	float current_frame = 0.0;

	const glm::vec3 lightDir = glm::normalize(glm::vec3(-3.0f, -1.0f, 0.0f));
	glm::vec3 lightPos = -lightDir * 1000.0f;

	// Frustum should be scaled with terrain size //
	glm::mat4 lightProjection = glm::ortho(-1024.0f, 1024.0f, -1024.0f, 1024.0f, 1.0f, 2000.0f);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
	Shader skybox_shader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");
	Shader depth_shader("shaders/depth_vertex.glsl", "shaders/depth_fragment.glsl");
	Shader debug_line_shader("shaders/debugLineVertex.glsl", "shaders/debugLineFragment.glsl");

	// Move all of this somewhere out of this module , maybe into render // 
	GLuint frustumVAO, frustumVBO, frustumEBO;

	unsigned int frustumIndices[] = {
    	0, 1,  1, 3,  3, 2,  2, 0, 
    	4, 5,  5, 7,  7, 6,  6, 4, 
    	0, 4,  1, 5,  2, 6,  3, 7  
	};

	glGenVertexArrays(1, &frustumVAO);
	glGenBuffers(1, &frustumVBO);
	glGenBuffers(1, &frustumEBO);

	glBindVertexArray(frustumVAO);

	glBindBuffer(GL_ARRAY_BUFFER, frustumVBO);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, frustumEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frustumIndices), frustumIndices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glBindVertexArray(0);
	// -----------------------------------------------------------------------------------//

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
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);

			glm::mat4 model(1.0f);

			depth_shader.use();
			depth_shader.set_mat4("lightSpaceMatrix", lightSpaceMatrix);
			depth_shader.set_mat4("model", model);

			terrain->draw();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glViewport(0, 0, int(window_width), int(window_height));
			glBindTexture(GL_TEXTURE_2D, depthMap);

			// Turn this into debug option ---------------------------------------------//
			DrawLightFrustum(frustumVAO, frustumVBO, lightSpaceMatrix, debug_line_shader);
			DrawLightMarker(lightPos, lightDir, debug_line_shader);
			// -------------------------------------------------------------------------//

			shader.use();
			shader.set_mat4("model", model);
			
			glm::mat4 view = camera.get_view_mat();
			shader.set_mat4("view", view);

			glm::mat4 projection = glm::perspective(glm::radians(45.0f), window_width/window_height, 0.1f, camera.view_distance);
			shader.set_mat4("projection", projection);

			shader.set_mat4("lightSpaceMatrix", lightSpaceMatrix);

			shader.set_float("min_y", terrain->min_height);
			shader.set_float("max_y", terrain->max_height);
			shader.set_bool("show_normals", show_normals);
			shader.set_bool("calculate_lighting", calculate_lighting);
			shader.set_vec3("lightDir", lightDir);

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
