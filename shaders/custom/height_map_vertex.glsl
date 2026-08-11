#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform float uWorldSize;
out vec2 uv;

void main(){
	uv = (aTexCoords - 0.5) * uWorldSize;
	gl_Position = vec4(aPos, 0.0, 1.0);
}
