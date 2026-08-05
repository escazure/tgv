#pragma once
#include "core.h"
#include "state.h"

void render_gui(AppState& state);
void init_skybox();
void init_fbo(unsigned int& depthMapFBO, unsigned int& depthMap);
void init_light_frustum();
void draw_skybox(Shader shader);
void DrawLightFrustum(const glm::mat4& lightSpaceMatrix, Shader& debugLineShader);
void DrawLightMarker(glm::vec3 lightPos, glm::vec3 lightDir, Shader& debugLineShader);
