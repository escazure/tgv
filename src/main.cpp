#include "core.h"

int main(){
	GLFWwindow* window = init();
	std::cout << "Got window handle\n";
	runStartUI(window);
	run(window);
	shutdown(window);
}
