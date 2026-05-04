#version 330 core

in vec3 Color;
in float normalized_y;
in vec3 Normal;

out vec4 FragColor;

uniform bool show_normals;

void main(){
	vec3 color = Color;
	vec3 normal = normalize(Normal);
	if(normalized_y < 0.3){
		color = vec3(0.8, 0.8, 0.5);
	}
	else if(normalized_y < 0.5){
		color = vec3(0.1, 0.7, 0.1);
	}
	else if(normalized_y < 0.9){
		color = vec3(0.5, 0.5, 0.5);
	}
	if(show_normals) color = normal * 0.5 + 0.5;
	FragColor = vec4(color, 1.0);
}

