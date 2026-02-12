#include "core.h"

Renderer rend;

GLFWwindow* init(float* vertices, unsigned int size, unsigned int* indices, unsigned int isize){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "tgv", NULL, NULL);

	glfwMakeContextCurrent(window);

	/* Input initialization*/

	if(gl3wInit() != 0)
		std::cerr << "ERROR: Failed to init gl3w" << std::endl;

	rend.init_renderer(vertices, size, indices, isize, true);

	glEnable(GL_DEPTH_TEST);

	return window;
}

void run(GLFWwindow* window, float* vertices, unsigned int size){
	float delta_time = 0.0;
	float last_frame = 0.0;
	float current_frame = 0.0;

	Shader shader("vertex.glsl", "fragment.glsl");
	shader.use();
	
	glm::mat4 model(1.0f);
	shader.set_mat4("model", model);

	while(!glfwWindowShouldClose(window)){
		current_frame = glfwGetTime();
		delta_time = current_frame - last_frame;	
		last_frame = current_frame;

		process_input(window);

		glm::mat4 view(1.0f);
		view = glm::lookAt(glm::vec3(0.0, 0.0, 5.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		shader.set_mat4("view", view);

		glm::mat4 projection(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)1920/(float)1080, 0.1f, 100.0f);
		shader.set_mat4("projection", projection);

		glClearColor(0.2, 0.3, 0.3, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		rend.render(3);	// Render three vertices of test triangle

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void shutdown(GLFWwindow* window){
	glfwDestroyWindow(window);	
	glfwTerminate();
}

void process_input(GLFWwindow* window){

}
