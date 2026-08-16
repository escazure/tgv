#include "helper.h"
#include "render.h"

bool g_is_capturing = false;

void mouse_callback(double xpos, double ypos, Camera& _camera){
	static bool first_mouse = true;
	static float lastx = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
	static float lasty = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;
	if(!g_is_capturing){
		first_mouse = true;
		return;
	}
		
	float yawt = _camera._yaw, pitcht = _camera._pitch;

	if(first_mouse){
		lastx = (float)xpos;
		lasty = (float)ypos;
		first_mouse = false;
	}

	float xoffset = xpos - lastx;
	float yoffset = lasty - ypos;
	lastx = xpos;
	lasty = ypos;

	_camera.processMouseMove(xoffset, yoffset);
}

void key_callback(GLFWwindow* window, int key, int action){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		if(ImGui::GetIO().WantCaptureKeyboard) return;
		state.show_ui = !state.show_ui;
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

	glm::vec3 moveInput(0.0f);

	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveInput.z += 1.0f;
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveInput.z -= 1.0f;

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveInput.x += 1.0f;
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveInput.x -= 1.0f;

	if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) moveInput.y += 1.0f;
	if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) moveInput.y -= 1.0f;

	state.camera->move(moveInput, delta_time);
}

std::string wrap_user_input(const std::string& function_body, const std::string& function_name){
	std::string result;		
	result = "float " + function_name + "(vec2 uv){\n" + function_body + "\n}";
	return result;
}

void build_shader(const std::string& udf, const std::string& udf_name, const std::string& shader_name){
	std::ifstream is(std::string(PROJECT_SHADERS_DIR) + std::string("/heightMapLib/generation_lib.glsl"));
	std::stringstream buffer;
	buffer << is.rdbuf();
	std::string lib = buffer.str();

	std::ofstream os(std::string(PROJECT_SHADERS_DIR) + std::string("/heightMap/") + shader_name + std::string(".comp"));	
	os << "#version 460 core\n"
	      "layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;\n"
		  "layout (binding = 0, r32f) writeonly uniform image2D uHeightMap;\n\n"
		  "uniform float uWorldSize;\n\n"
		  "uniform int uSeed;\n\n";

	os << lib << "\n\n";

	os << "/*** USER DEFINED FUNCTION BEGIN ***/\n";
	os << udf << "\n";
	os << "/*** USER DEFINED FUNCTION END ***/\n\n";

	os << "void main(){\n"
		  " ivec2 dstCoords = ivec2(gl_GlobalInvocationID.xy);\n"
		  " vec2 uv = vec2(dstCoords) - uWorldSize * 0.5;\n"
		  " imageStore(uHeightMap, dstCoords, vec4(" << udf_name << "(uv), 0.0, 0.0, 0.0));\n"
		  "}";
}

void getMinMaxHeight(Shader& shader, AppState& state, Texture &heightMap, unsigned int width, unsigned int height) {
    int maxMipLevels = 1 + std::floor(std::log2(std::max(width, height)));

    Texture minMaxTexture;
	minMaxTexture.create(GL_TEXTURE_2D, maxMipLevels, GL_RG32F, width, height);

	Sampler nearestClamp;
	nearestClamp.create(GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);

	Sampler linearClamp;
	linearClamp.create(GL_LINEAR, GL_LINEAR);

    shader.use();

    int currentWidth = width;
    int currentHeight = height;

    for(int srcMip = -1; srcMip < maxMipLevels - 1; srcMip++){
        int dstMip = srcMip + 1;
        int dstWidth = std::max(1, currentWidth / 2);
        int dstHeight = std::max(1, currentHeight / 2);

        if(srcMip == -1){
			heightMap.bind(0);
			linearClamp.bind(0);
            shader.set_int("uSrcMipLevel", -1);
        }
		else{
			minMaxTexture.bind(0);
			nearestClamp.bind(0);
            shader.set_int("uSrcMipLevel", srcMip);
        }

        glBindImageTexture(1, minMaxTexture._id, dstMip, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

        int groupsX = (dstWidth + 7) / 8;
        int groupsY = (dstHeight + 7) / 8;
        glDispatchCompute(groupsX, groupsY, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

        currentWidth = dstWidth;
        currentHeight = dstHeight;
    }

    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

    float result[2] = { 0.0f, 0.0f };

    glGetTextureSubImage(
        minMaxTexture._id,
        maxMipLevels - 1,
        0, 0, 0,         
        1, 1, 1,         
        GL_RG, GL_FLOAT,
        sizeof(result),
        result
    );

    state.min_height = result[0];
    state.max_height = result[1];
}

void debugMessageCallback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char *msg, const void *data){
	std::string _source;
	std::string _type;
	std::string _severity;

	if(severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    switch(source){
        case GL_DEBUG_SOURCE_API:
        	_source = "API";
        	break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        	_source = "WINDOW SYSTEM";
        	break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
        	_source = "SHADER COMPILER";
        	break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
        	_source = "THIRD PARTY";
        	break;

        case GL_DEBUG_SOURCE_APPLICATION:
        	_source = "APPLICATION";
        	break;

        default:
        	_source = "UNKNOWN";
        	break;
    }

    switch(type){
        case GL_DEBUG_TYPE_ERROR:
        	_type = "ERROR";
        	break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        	_type = "DEPRECATED BEHAVIOR";
        	break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        	_type = "UNDEFINED BEHAVIOR";
        	break;

        case GL_DEBUG_TYPE_PORTABILITY:
        	_type = "PORTABILITY";
        	break;

        case GL_DEBUG_TYPE_PERFORMANCE:
        	_type = "PERFORMANCE";
        	break;

        case GL_DEBUG_TYPE_OTHER:
        	_type = "OTHER";
        	break;

        case GL_DEBUG_TYPE_MARKER:
        	_type = "MARKER";
        	break;

        default:
        	_type = "UNKNOWN";
        	break;
    }

    switch(severity){
        case GL_DEBUG_SEVERITY_HIGH:
        	_severity = "HIGH";
        	break;

        case GL_DEBUG_SEVERITY_MEDIUM:
        	_severity = "MEDIUM";
        	break;

        case GL_DEBUG_SEVERITY_LOW:
        	_severity = "LOW";
        	break;

        default:
        	_severity = "UNKNOWN";
        	break;
    }
	
	std::cout << id << ": " << _type << " of " << _severity << ", raised from " << _source << ": " << msg << "\n";
}
