#include "render.h"
#include "ui.h"
#include "helper.h"

unsigned int skyboxVAO, skyboxVBO;	

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

	glCreateVertexArrays(1, &skyboxVAO);
	glCreateBuffers(1, &skyboxVBO);

	glNamedBufferData(skyboxVBO, sizeof(skyboxVert), skyboxVert, GL_STATIC_DRAW);
	glVertexArrayVertexBuffer(skyboxVAO, 0, skyboxVBO, 0, 3 * sizeof(float));

	glEnableVertexArrayAttrib(skyboxVAO, 0);
	glVertexArrayAttribFormat(skyboxVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(skyboxVAO, 0, 0);
}

void render_skybox(Shader shader, const glm::vec3& lightDir){
	shader.use();

	glm::mat4 view = glm::mat4(glm::mat3(state.camera->getViewMat()));
	shader.set_mat4("uView", view);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->_viewDistance);
	shader.set_mat4("uProjection", projection);

	shader.set_vec3("uLightDir", lightDir);

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

        glCreateVertexArrays(1, &quadVAO);
        glCreateBuffers(1, &quadVBO);

        glNamedBufferData(quadVBO, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, 4 * sizeof(float));

        glEnableVertexArrayAttrib(quadVAO, 0);
		glVertexArrayAttribFormat(quadVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(quadVAO, 0, 0);

        glEnableVertexArrayAttrib(quadVAO, 1);
		glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
		glVertexArrayAttribBinding(quadVAO, 1, 0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
