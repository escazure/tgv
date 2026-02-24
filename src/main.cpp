#include "core.h"

int main(){
	GLFWwindow* window = init();
	run(window);
	shutdown(window);
}
