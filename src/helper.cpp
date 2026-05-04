#include "helper.h"

bool g_is_capturing = false;

void mouse_callback(double xpos, double ypos, Camera& _camera){
	static bool first_mouse = true;
	static float lastx = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
	static float lasty = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
	if(!g_is_capturing){
		first_mouse = true;
		return;
	}
		
	float yawt = _camera.yaw, pitcht = _camera.pitch;

	if(first_mouse){
		lastx = (float)xpos;
		lasty = (float)ypos;
		first_mouse = false;
	}

	float xoffset = xpos - lastx;
	float yoffset = lasty - ypos;
	lastx = xpos;
	lasty = ypos;

	_camera.process_mouse_mov(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int action){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		if(ImGui::GetIO().WantCaptureKeyboard) return;
		g_is_capturing = !g_is_capturing;

		glfwSetInputMode(window, GLFW_CURSOR, g_is_capturing ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
}

void mouse_callback_wrapper(GLFWwindow* window, double xpos, double ypos){
	mouse_callback(xpos, ypos, camera);
}

void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods){
	key_callback(window, key, action);
}

void process_input(GLFWwindow* window, float delta_time){
	if(!g_is_capturing) return;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)	
		camera.move_forward(delta_time);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.move_back(delta_time);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.move_left(delta_time);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.move_right(delta_time);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.move_up(delta_time);
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.move_down(delta_time);
}

unsigned int loadCubeMap(std::vector<std::string>& faces){
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	unsigned char* data;
	for(unsigned int i = 0; i < faces.size(); i++){
		data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if(data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, nrChannels == 4 ? GL_RGBA : GL_RGB, width, height, 0, nrChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			std::cerr << "ERROR: Failed to load texture for cubemap [" << faces[i] << "]\n";
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
