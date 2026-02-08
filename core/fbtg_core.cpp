#include "fbtg_core.h"

GLFWwindow* init_subsystems(float* vertices, u32 width, u32 length){
    glfwSetErrorCallback(error_callback);

    if(!glfwInit()){
        std::cout << "Failed to init glfw\n";
    }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
	
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(mode -> width, mode -> height,"Test",NULL,NULL);
    if(!window){
        std::cout << "Failed to create window\n";
    }

    glfwMakeContextCurrent(window);

	if(gl3wInit() != 0) {
		std::cout << "Failed to init gl3w\n";
		return 0;
	}

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

    int w, h;
	glfwGetFramebufferSize(window, &w, &h);
    framebuffer_size_callback(window, w, h);

	render_module::init(vertices, width, length);

	return window;	
}

void run(GLFWwindow* _window){
	while(!glfwWindowShouldClose(_window)){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render_module::render();
		glfwSwapBuffers(_window);
		glfwPollEvents();
		Input::update(_window);
		glClearColor(0.3,0.5,0.7,0.7);
		glClear(GL_COLOR_BUFFER_BIT);
	}	
}

void shutdown(void** _handle, GLFWwindow* _window){
	dlclose(_handle);
    glfwDestroyWindow(_window);
    glfwTerminate();
}
