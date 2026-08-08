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
	mouse_callback(xpos, ypos, *state.camera);
}

void key_callback_wrapper(GLFWwindow* window, int key, int scancode, int action, int mods){
	key_callback(window, key, action);
}

void process_input(GLFWwindow* window, float delta_time){
	if(!g_is_capturing) return;
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)	
		state.camera->move_forward(delta_time);
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		state.camera->move_back(delta_time);
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		state.camera->move_left(delta_time);
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		state.camera->move_right(delta_time);
	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		state.camera->move_up(delta_time);
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		state.camera->move_down(delta_time);
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

std::vector<glm::vec3> GetLightFrustumCornersWorldSpace(const glm::mat4& lightSpaceMatrix) {
    glm::mat4 invLightSpace = glm::inverse(lightSpaceMatrix);
    
    std::vector<glm::vec3> corners;
    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                glm::vec4 pt = invLightSpace * glm::vec4(
                    x * 2.0f - 1.0f,
                    y * 2.0f - 1.0f,
                    z * 2.0f - 1.0f,
                    1.0f
                );
                corners.push_back(glm::vec3(pt) / pt.w);
            }
        }
    }
    return corners;
}

void calculateTightLightProjection(float maxH, float minH, float halfLen, glm::mat4& lightProjection, glm::mat4& lightView, glm::vec3 lightDir){
	glm::vec3 terrainCenter = glm::vec3(0.0f, (minH + maxH) * 0.5f, 0.0f);

	float sceneRadius = glm::length(glm::vec3(halfLen, (maxH - minH) * 0.5f, halfLen));
	glm::vec3 lightPos = terrainCenter - glm::normalize(lightDir) * sceneRadius * 1.5f;

	glm::vec3 corners[8] = {
    	glm::vec3(-halfLen, minH, -halfLen),
    	glm::vec3( halfLen, minH, -halfLen),
    	glm::vec3(-halfLen, maxH, -halfLen),
    	glm::vec3( halfLen, maxH, -halfLen),
    	glm::vec3(-halfLen, minH,  halfLen),
    	glm::vec3( halfLen, minH,  halfLen),
    	glm::vec3(-halfLen, maxH,  halfLen),
    	glm::vec3( halfLen, maxH,  halfLen)
	};

	float minX =  std::numeric_limits<float>::max();
	float maxX = -std::numeric_limits<float>::max();
	float minY =  std::numeric_limits<float>::max();
	float maxY = -std::numeric_limits<float>::max();
	float minZ =  std::numeric_limits<float>::max();
	float maxZ = -std::numeric_limits<float>::max();

	for (int i = 0; i < 8; ++i) {
	    glm::vec4 pt = lightView * glm::vec4(corners[i], 1.0f);
	    minX = std::min(minX, pt.x);
	    maxX = std::max(maxX, pt.x);
	    minY = std::min(minY, pt.y);
	    maxY = std::max(maxY, pt.y);
	    minZ = std::min(minZ, pt.z);
	    maxZ = std::max(maxZ, pt.z);
	}

	float padding = 10.0f;
	lightProjection = glm::ortho(
	    minX - padding, maxX + padding,
	    minY - padding, maxY + padding,
	    -maxZ - padding, -minZ + padding
	);
}

std::string wrap_user_input(const std::string& function_body, const std::string& function_name){
	std::string result;		
	result = "float " + function_name + "(){\n" + function_body + "\n}";
	return result;
}

unsigned int build_shader(const std::string& udf, const std::string& udf_name, const std::string& shader_name){
	std::string functions_dir = PROJECT_FUNCTIONS_DIR;

	std::ifstream is(functions_dir + "/generation_lib.glsl");
	if(!is.is_open()) return FAILED_TO_OPEN_FILE;
	std::stringstream buffer;
	buffer << is.rdbuf();
	std::string lib = buffer.str();

	std::ofstream os(functions_dir + "/" + shader_name + ".glsl");	
	if(!os.is_open()) return FAILED_TO_OPEN_FILE;
	os << "#version 330 core\n"
	      "in vec2 uv;\n"
		  "out float FragColor;\n\n"
		  "uniform int uSeed;\n\n";

	os << lib << "\n\n";

	os << "/*** USER DEFINED FUNCTION BEGIN ***/\n";
	os << udf << "\n";
	os << "/*** USER DEFINED FUNCTION END ***/\n\n";

	os << "void main(){\n"
		  " FragColor = " << udf_name << "();\n"
		  "}";

	if(os.fail()){
		return FAILED_TO_WRITE_FILE;
	}	

	return SUCCESS;
}
