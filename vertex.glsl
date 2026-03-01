#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 Color;
out float normalized_y;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float min_y;
uniform float max_y;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	Color = vec3(1.0, 1.0, 1.0);
	normalized_y = (aPos.y - min_y)/(max_y - min_y);
}

