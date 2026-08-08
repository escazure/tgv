#pragma once
#include "core.h"
#include "state.h"

void render_gui(AppState& state);
void init_skybox();
void init_fbo(unsigned int& fbo, unsigned int& textureMap, unsigned int width, unsigned int height, unsigned int numberOfChannels, bool isDepth);
void draw_skybox(Shader shader);
void render_quad();
void resize_fbo(unsigned int textureMap, unsigned int newWidth, unsigned int newHeight, unsigned int numberOfChannels, bool isDepth);
