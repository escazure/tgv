#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
	glm::vec3 pos, front, up, right, world_up;
	float speed, yaw, pitch, fov, height, sensitivity, view_distance;

	Camera(const glm::vec3 &_pos, float _speed, float _sens, float _view_distance = 1000){
		pos = _pos; 
		speed = _speed;
		sensitivity = _sens;

		front = glm::vec3(0.0, 0.0, -1.0);
		world_up = glm::vec3(0.0, 1.0, 0.0);

		yaw = -90.0;
		pitch = 0.0;
		fov = 45.0;

		height = pos.y;

		view_distance = _view_distance; 

		update_dir();
	}

	void update_dir(){
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(direction);

		right = glm::normalize(glm::cross(front, world_up));
		up = glm::normalize(glm::cross(right, front));
	}

	void move_forward(float delta){
		pos += speed * front * delta;	
	}
	void move_back(float delta){
		pos -= speed * front * delta;	
	}
	void move_right(float delta){
		pos += speed * right * delta;	
	}
	void move_left(float delta){
		pos -= speed * right * delta;	
	}
	void move_up(float delta){
		pos += speed * world_up * delta;	
	}
	void move_down(float delta){
		pos -= speed * world_up * delta;	
	}

	void process_mouse_mov(float xoffset, float yoffset){
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if(pitch > 75.0)
			pitch = 75.0;
		if(pitch < -75.0)
			pitch = -75.0;

		update_dir();
	}

	glm::mat4 get_view_mat(){
		return glm::lookAt(pos, pos + front, up);
	}
};
