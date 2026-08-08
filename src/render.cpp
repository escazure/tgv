#include "render.h"
#include "ui.h"
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
	skybox_cubemap = load_cube_map(faces);
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

void render_quad(){
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
