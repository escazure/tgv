#pragma once
#include "gl3w.h"

struct Sampler {
	unsigned int _id = 0;

public:
	~Sampler(){
		glDeleteSamplers(1, &_id);
	}

	void create(unsigned int minFilter, unsigned int magFilter, unsigned int wrapS = GL_CLAMP_TO_EDGE, unsigned int wrapT = GL_CLAMP_TO_EDGE, unsigned int wrapR = GL_CLAMP_TO_EDGE){
		glCreateSamplers(1, &_id);

		setParameter(GL_TEXTURE_MIN_FILTER, minFilter);
		setParameter(GL_TEXTURE_MAG_FILTER, magFilter);
		setParameter(GL_TEXTURE_WRAP_S, wrapS);
		setParameter(GL_TEXTURE_WRAP_T, wrapT);
		setParameter(GL_TEXTURE_WRAP_R, wrapR);
	}

	void bind(unsigned int unit) const {
		glBindSampler(unit, _id);
	}

	void unbind(unsigned int unit) const {
		glBindSampler(unit, 0);
	}

	void setParameter(unsigned int pname, unsigned int param) const {
		glSamplerParameteri(_id, pname, param);
	}
};
