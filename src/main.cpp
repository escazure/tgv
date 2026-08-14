#include "core.h"

int main(int argc, char** argv){
	bool useIntegrated = false;
	for(int i = 1; i < argc; i++){
		if(std::strcmp(argv[i], "--debug") == 0) state.debug_mode = true;
		else if(std::strcmp(argv[i], "--log") == 0) state.logging = true;
		else if(std::strcmp(argv[i], "--integrated") == 0) useIntegrated = true;
	}

	if(!useIntegrated && std::system("nvidia-smi --query-gpu=name --format=csv,noheader > /dev/null 2>&1") == 0)
		setenv("__NV_PRIME_RENDER_OFFLOAD", "1", 1);

	GLFWwindow* window = init();
	run(window);
	shutdown(window);
}
