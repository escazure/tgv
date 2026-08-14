#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
public:
	glm::vec3 _pos, _front, _up, _right, _worldUp;
	float _speed, _yaw, _pitch, _fov, _height, _sensitivity, _viewDistance;

	Camera(const glm::vec3 &pos, float speed, float sens, float viewDistance = 3000){
		_pos = pos; 
		_speed = speed;
		_sensitivity = sens;

		_front = glm::vec3(0.0, 0.0, -1.0);
		_worldUp = glm::vec3(0.0, 1.0, 0.0);

		_yaw = 0.0;
		_pitch = 0.0;
		_fov = 45.0;

		_height = _pos.y;

		_viewDistance = viewDistance; 

		pUpdateDir();
	}

	void move(const glm::vec3 &inputDir, float delta){
		if(inputDir == glm::vec3(0.0f)) return;

		glm::vec3 moveFront = glm::normalize(glm::vec3(_front.x, 0.0f, _front.z));
		glm::vec3 moveDir = (moveFront * inputDir.z) + (_right * inputDir.x) + (_worldUp * inputDir.y); 
		moveDir = glm::normalize(moveDir);
		_pos += moveDir * _speed * delta;

		if(inputDir.y == 0.0f) _pos.y = _height;
		else _height = _pos.y;
	}

	void processMouseMove(float xoffset, float yoffset){
		xoffset *= _sensitivity;
		yoffset *= _sensitivity;

		_yaw += xoffset;
		_pitch += yoffset;

		if(_pitch > 75.0)
			_pitch = 75.0;
		if(_pitch < -75.0)
			_pitch = -75.0;

		pUpdateDir();
	}

	glm::mat4 getViewMat(){
		return glm::lookAt(_pos, _pos + _front, _up);
	}

private:
	void pUpdateDir(){
		glm::vec3 direction;
		direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		direction.y = sin(glm::radians(_pitch));
		direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
		
		_front = glm::normalize(direction);
		_right = glm::normalize(glm::cross(_front, _worldUp));
		_up = glm::normalize(glm::cross(_right, _front));
	}
};
