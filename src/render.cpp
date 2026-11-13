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

	glm::mat4 v = glm::mat4(glm::mat3(state.camera->getViewMat()));
	shader.set_mat4("uView", v);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 0.1f, state.camera->_viewDistance);
	shader.set_mat4("uProjection", projection);

	shader.set_vec3("uLightDir", lightDir);

	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void render_water_plane(float size, float level, Shader shader, const glm::vec3& lightDir){
	shader.use();
	
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(0.0, level, 0.0));
	model = glm::scale(model, glm::vec3(size * 0.5f));
	shader.set_mat4("uModel", model);

	glm::mat4 view = state.camera->getViewMat();
	shader.set_mat4("uView", view);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 1.0f, state.camera->_viewDistance);
	shader.set_mat4("uProjection", projection);

	shader.set_vec3("uLightDir", lightDir);
	shader.set_vec3("uCameraPos", state.camera->_pos);

	static unsigned int quadVAO = 0;
    static unsigned int quadVBO = 0;

    if(quadVAO == 0){
        float quadVertices[] = {
            -1.0f, 0.0f,  1.0f,  0.0f, 1.0f,
             1.0f, 0.0f, -1.0f,  1.0f, 0.0f,
            -1.0f, 0.0f, -1.0f,  0.0f, 0.0f,

            -1.0f, 0.0f,  1.0f,  0.0f, 1.0f,
             1.0f, 0.0f,  1.0f,  1.0f, 1.0f,
             1.0f, 0.0f, -1.0f,  1.0f, 0.0f,
        };

        glCreateVertexArrays(1, &quadVAO);
        glCreateBuffers(1, &quadVBO);

        glNamedBufferData(quadVBO, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glVertexArrayVertexBuffer(quadVAO, 0, quadVBO, 0, 5 * sizeof(float));

        glEnableVertexArrayAttrib(quadVAO, 0);
		glVertexArrayAttribFormat(quadVAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(quadVAO, 0, 0);

        glEnableVertexArrayAttrib(quadVAO, 1);
		glVertexArrayAttribFormat(quadVAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
		glVertexArrayAttribBinding(quadVAO, 1, 0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void render_terrain(Shader shader, const glm::vec3& lightDir, const glm::mat4& view, const glm::vec4& clipPlane){
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
	shader.set_vec4("uClipPlane", clipPlane);

	glm::mat4 model(1.0f);
	shader.set_mat4("uModel", model);
			
	shader.set_mat4("uView", view);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), state.window_width/state.window_height, 1.0f, state.camera->_viewDistance);
	shader.set_mat4("uProjection", projection);

	state.terrain->draw();
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
             1.0f, 	1.0f,  1.0f, 1.0f 
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
