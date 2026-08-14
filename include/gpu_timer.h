#pragma once
#include "gl3w.h"

struct Timer {
	std::string _name;
	unsigned int _query = 0;
	bool _has_ended = false;

	Timer(){
		_name = "_DEFAULT_";
		glGenQueries(1, &_query);
	}

	Timer(const std::string& name){
		_name = name;
		glGenQueries(1, &_query);
	}

	~Timer(){
		if(_query)
			glDeleteQueries(1, &_query);
	}
	
	void begin(){
		glBeginQuery(GL_TIME_ELAPSED, _query);
		_has_ended = false;
	}

	void end(){
		glEndQuery(GL_TIME_ELAPSED);
		_has_ended = true;
	}

	bool isAvailable() const {
		if(!_has_ended) return false;

		int a = 0;
		glGetQueryObjectiv(_query, GL_QUERY_RESULT_AVAILABLE, &a);
		return a;
	}

	double getMilli(){
		unsigned long e = 0;
		glGetQueryObjectui64v(_query, GL_QUERY_RESULT, &e);

		_has_ended = false;
		return double(e) / 1e6;
	}
};
