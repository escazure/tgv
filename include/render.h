#pragma once
#include "core.h"
#include "state.h"

void render_gui(AppState& state);
void render_quad();
void draw_skybox(Shader shader);

void init_skybox();
void init_fbo(unsigned int& fbo, unsigned int& textureMap, unsigned int width, unsigned int height, unsigned int numberOfChannels, unsigned int mipMapLevels, bool interpolate);
void init_texture(unsigned int& textureMap, unsigned int width, unsigned int height, unsigned int numberOfChannels, unsigned int mipMapLevels, bool interpolate);

void resize_fbo_attachment(unsigned int fbo, unsigned int& textureMap, unsigned int newWidth, unsigned int newHeight, unsigned int numberOfChannels, unsigned int mipMapLevels, bool interpolate);
void resize_texture(unsigned int& textureMap, unsigned int width, unsigned int height, unsigned int numberOfChannels, unsigned int mipMapLevels, bool interpolate);
