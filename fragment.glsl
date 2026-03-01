#version 330 core

in vec3 Color;
in float normalized_y;

out vec4 FragColor;

void main(){
	FragColor = vec4(Color, 1.0);
	if(normalized_y < 0.3){
		FragColor = vec4(0.8, 0.8, 0.5, 1.0);
	}
	else if(normalized_y < 0.5){
		FragColor = vec4(0.1, 0.7, 0.1, 1.0);
	}
	else if(normalized_y < 0.9){
		FragColor = vec4(0.5, 0.5, 0.5, 1.0);
	}
}

