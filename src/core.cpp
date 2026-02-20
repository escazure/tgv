#include "core.h"

Renderer rend;
Camera camera(glm::vec3(0.0, 10.0, 0.0), 10.0, 0.07);
float lastx, lasty, delta_time;
bool first_mouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	float yawt = camera.yaw, pitcht = camera.pitch;

	if(first_mouse){
		lastx = (float)xpos;
		lasty = (float)ypos;
		first_mouse = false;
	}

	float xoffset = xpos - lastx;
	float yoffset = lasty - ypos;
	lastx = xpos;
	lasty = ypos;

	camera.process_mouse_mov(xoffset, yoffset);
}

void process_input(GLFWwindow* window){
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

GLFWwindow* init(float* vertices, unsigned int size, unsigned int* indices, unsigned int isize){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "tgv", NULL, NULL);

	glfwMakeContextCurrent(window);

	lastx = 1980/2;
	lasty = 1080/2;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	if(gl3wInit() != 0)
		std::cerr << "ERROR: Failed to init gl3w" << std::endl;

	rend.init_renderer(vertices, size, indices, isize, true);

	glEnable(GL_DEPTH_TEST);

	return window;
}

void run(GLFWwindow* window, float* vertices, unsigned int size){
	delta_time = 0.0;
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
		view = camera.get_view_mat();
		shader.set_mat4("view", view);

		glm::mat4 projection(1.0f);
		projection = glm::perspective(glm::radians(45.0f), (float)1920/(float)1080, 0.1f, 500.0f);
		shader.set_mat4("projection", projection);

		glClearColor(0.2, 0.6, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		rend.render(size);	// Render three vertices of test triangle

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void shutdown(GLFWwindow* window){
	glfwDestroyWindow(window);	
	glfwTerminate();
}
