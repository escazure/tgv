#pragma once

#include "core.h"

#define MAX_EXPONENT 13
#define MIN_EXPONENT 5
#define MIN_STEP_EXPONENT 0
#define MAX_STEP_EXPONENT 3
#define MIN_CHUNK_EXPONENT 5
#define MAX_CHUNK_EXPONENT 8
#define MIN_CAMERA_SPEED 10.0f
#define MAX_CAMERA_SPEED 100.0f
#define MIN_CAMERA_VIEW_DISTANCE 100.0f
#define MAX_CAMERA_VIEW_DISTANCE 10000.0f

#define SHADOW_WIDTH 2048 
#define SHADOW_HEIGHT 2048 

void render_gui();
void init_skybox();
void init_fbo(unsigned int& depthMapFBO, unsigned int& depthMap);
void init_light_frustum();
void draw_skybox(Shader shader);
void DrawLightFrustum(const glm::mat4& lightSpaceMatrix, Shader& debugLineShader);
void DrawLightMarker(glm::vec3 lightPos, glm::vec3 lightDir, Shader& debugLineShader);
