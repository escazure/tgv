#pragma once
#include <iostream>
#include "gl3w.h"

struct Texture {
	unsigned int _id = 0;
	unsigned int _target = 0;
	unsigned int _internalFormat = 0;
	unsigned int _mipMapLevels = 1;
	unsigned int _width = 0;
	unsigned int _height = 0;
	unsigned int _length = 0;

public: 
	~Texture(){
		glDeleteTextures(1, &_id);	
	}

	void create(unsigned int target, unsigned int mipMapLevels, unsigned int internalFormat, unsigned int width, unsigned int height = 1, unsigned int length = 1){
		_target = target;
		_internalFormat = internalFormat;
		_mipMapLevels = mipMapLevels;
		_width = width;
		_height = height;
		_length = length;

		glCreateTextures(_target, 1, &_id);
		pSpecifyTextureStorage();
	}

	void bind(unsigned int unit) const {
		glBindTextureUnit(unit, _id);
	}

	void bindAsImage(unsigned int unit, unsigned int mipMapLevel = 0) const {
		glBindImageTexture(unit, _id, mipMapLevel, GL_FALSE, 0, GL_WRITE_ONLY, _internalFormat);
	}

	void attach(unsigned int fbo, unsigned int attachment, unsigned int mipMapLevel = 0) const {
		glNamedFramebufferTexture(fbo, attachment, _id, mipMapLevel);
	}

	void resize(unsigned int mipMapLevels, unsigned int width, unsigned int height = 1, unsigned int length = 1){
		if(_id != 0)
			glDeleteTextures(1, &_id);

		_mipMapLevels = mipMapLevels;
		_width = width;
		_height = height;
		_length = length;

		glCreateTextures(_target, 1, &_id);
		pSpecifyTextureStorage();
	}

private:
	void pSpecifyTextureStorage(){
		switch(_target){
			case GL_TEXTURE_1D:
				glTextureStorage1D(_id, _mipMapLevels, _internalFormat, _width);
				break;

			case GL_TEXTURE_2D:
			case GL_TEXTURE_1D_ARRAY:
			case GL_TEXTURE_RECTANGLE:
			case GL_TEXTURE_CUBE_MAP:
				glTextureStorage2D(_id, _mipMapLevels, _internalFormat, _width, _height);
				break;

			case GL_TEXTURE_3D:
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				glTextureStorage3D(_id, _mipMapLevels, _internalFormat, _width, _height, _length);
				break;
		}
	}
};
