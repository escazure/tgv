#pragma once
#include <vector>
#include "gl3w.h"

struct Texture {
	GLuint _id = 0;
	GLuint _target = 0;
	GLuint _internalFormat = 0;
	GLuint _pixelFormat = 0;
	unsigned int _mipMapLevels = 1;
	unsigned int _width = 0;
	unsigned int _height = 0;
	unsigned int _length = 0;
	unsigned int _channels = 0;

public: 
	~Texture(){
		glDeleteTextures(1, &_id);	
	}

	void create(GLuint target, unsigned int mipMapLevels, GLuint internalFormat, unsigned int width, unsigned int height = 1, unsigned int length = 1){
		_target = target;
		_internalFormat = internalFormat;
		_mipMapLevels = mipMapLevels;
		_width = width;
		_height = height;
		_length = length;


		glCreateTextures(_target, 1, &_id);
		pSpecifyTexturePixelFormat(_internalFormat);
		pSpecifyTextureStorage();

		_pPixelBuffer.resize(_width * _height * _channels);
	}

	void setWrap(GLuint wrapS, GLuint wrapT, GLuint wrapR = GL_REPEAT){
		glTextureParameteri(_id, GL_TEXTURE_WRAP_S, wrapS);	
		glTextureParameteri(_id, GL_TEXTURE_WRAP_T, wrapT);	
		if(_target == GL_TEXTURE_3D || _target == GL_TEXTURE_CUBE_MAP || _target == GL_TEXTURE_2D_ARRAY)
			glTextureParameteri(_id, GL_TEXTURE_WRAP_R, wrapR);
	}

	void setFilter(GLuint minFilter, GLuint magFilter){
		glTextureParameteri(_id, GL_TEXTURE_MIN_FILTER, minFilter);
		glTextureParameteri(_id, GL_TEXTURE_MAG_FILTER, magFilter);
	}

	void bind(unsigned int unit) const {
		glBindTextureUnit(unit, _id);
	}

	void bindAsImage(unsigned int unit, unsigned int mipMapLevel = 0) const {
		glBindImageTexture(unit, _id, mipMapLevel, GL_FALSE, 0, GL_WRITE_ONLY, _internalFormat);
	}

	void attach(GLuint fbo, GLuint attachment, unsigned int mipMapLevel = 0) const {
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
		pSpecifyTexturePixelFormat(_internalFormat);

		_pPixelBuffer.resize(_width * _height * _channels);
	}

	const float* pixels() const {
		unsigned int elementCount = _width * _height * _channels; 
		if(_pPixelBuffer.size() != elementCount)
			_pPixelBuffer.resize(elementCount);

		glGetTextureImage(_id, 0, _pixelFormat, GL_FLOAT, sizeof(float) * elementCount, _pPixelBuffer.data());
		return _pPixelBuffer.data();
	}

	void loadPixels(unsigned int width, unsigned int height, const float* data){
		glTextureSubImage2D(_id, 0, 0, 0, width, height, _pixelFormat, GL_FLOAT, data);	
	}

private:
	mutable std::vector<float> _pPixelBuffer;
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

	void pSpecifyTexturePixelFormat(unsigned int internalFormat){
		switch(internalFormat){
			case GL_DEPTH_COMPONENT32F:
			case GL_DEPTH_COMPONENT24:
				_pixelFormat = GL_DEPTH_COMPONENT;
				_channels = 1;
				break;
			case GL_R32F:
				_pixelFormat = GL_RED;
				_channels = 1;
				break;
			case GL_RG32F:
				_pixelFormat = GL_RG;
				_channels = 2;
				break;
			case GL_RGB32F:
				_pixelFormat = GL_RGB;
				_channels = 3;
				break;
			case GL_RGBA32F:
				_pixelFormat = GL_RGBA;
				_channels = 4;
				break;
		}
	}
};
