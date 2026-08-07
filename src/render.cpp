#include "render.h"
#include "ui.h"
#include "helper.h"

unsigned int skyboxVAO, skyboxVBO, skybox_cubemap;	
unsigned int frustumVAO, frustumVBO, frustumEBO;

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

void render_gui(AppState& state){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	UI::drawMenuBar(state);
    UI::drawGenerationPanel(state);
    UI::drawVisualizationPanel(state);
	
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

void init_fbo(unsigned int& fbo, unsigned int& textureMap, unsigned int width, unsigned int height, unsigned int numberOfChannels, bool isDepth){
	unsigned int container = GL_RGBA;
	unsigned int data = GL_RGBA32F;
	unsigned int attachment = GL_COLOR_ATTACHMENT0;
	switch(numberOfChannels){
		case 1: 
			container = GL_RED;
			data = GL_R32F;
			break;
		case 2:
			container = GL_RG;
			data = GL_RG32F;
			break;
		case 3:
			container = GL_RGB;
			data = GL_RGB32F;
			break;
		default:
			container = GL_RGBA;
			data = GL_RGBA32F;
			break;
	}

	if(isDepth){
		container = GL_DEPTH_COMPONENT;
		data = GL_DEPTH_COMPONENT32F;
		attachment = GL_DEPTH_ATTACHMENT;
	}

	glGenFramebuffers(1, &fbo);

	glGenTextures(1, &textureMap);
	glBindTexture(GL_TEXTURE_2D, textureMap);
	glTexImage2D(GL_TEXTURE_2D, 0, data, width, height, 0, container, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureMap, 0);
	if(isDepth){
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void resize_fbo(unsigned int textureMap, unsigned int newWidth, unsigned int newHeight, unsigned int numberOfChannels, bool isDepth) {
	unsigned int container = GL_RGBA;
	unsigned int data = GL_RGBA32F;

	switch(numberOfChannels){
		case 1: 
			container = GL_RED;
			data = GL_R32F;
			break;
		case 2:
			container = GL_RG;
			data = GL_RG32F;
			break;
		case 3:
			container = GL_RGB;
			data = GL_RGB32F;
			break;
		default:
			container = GL_RGBA;
			data = GL_RGBA32F;
			break;
	}

	if(isDepth){
		container = GL_DEPTH_COMPONENT;
		data = GL_DEPTH_COMPONENT32F;
	}

    glBindTexture(GL_TEXTURE_2D, textureMap);
    glTexImage2D(GL_TEXTURE_2D, 0, data, newWidth, newHeight, 0, container, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void init_light_frustum(){
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
}

void draw_skybox(Shader shader){
	shader.use();

	glm::mat4 view = glm::mat4(glm::mat3(state.camera->get_view_mat()));
	shader.set_mat4("view", view);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->view_distance);
	shader.set_mat4("projection", projection);

	shader.set_int("skybox", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap);

	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void DrawLightFrustum(const glm::mat4& lightSpaceMatrix, Shader& debugLineShader) {
    std::vector<glm::vec3> corners = GetLightFrustumCornersWorldSpace(lightSpaceMatrix);

    glBindBuffer(GL_ARRAY_BUFFER, frustumVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, corners.size() * sizeof(glm::vec3), corners.data());

    debugLineShader.use();
	glm::mat4 cameraProjection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->view_distance);
	glm::mat4 cameraView = state.camera->get_view_mat();
	glm::mat4 model(1.0f);
    debugLineShader.set_mat4("projection", cameraProjection);
    debugLineShader.set_mat4("view", cameraView);
    debugLineShader.set_mat4("model", model); 
    debugLineShader.set_vec3("color", glm::vec3(1.0f, 1.0f, 0.0f));

    glLineWidth(3.0f); 
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
	glm::mat4 cameraProjection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->view_distance);
	glm::mat4 cameraView = state.camera->get_view_mat();
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

void renderQuad(){
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO = 0;

    if(quadVAO == 0){
        float quadVertices[] = {
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f 
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
