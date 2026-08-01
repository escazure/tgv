#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;
out vec3 WorldPos;
out float normalized_y;
out vec4 FragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main(){
	vec4 worldPos = model * vec4(aPos, 1.0);
	WorldPos = vec3(worldPos);
	gl_Position = projection * view * worldPos;
	Normal = aNormal;
	Position = vec3(model * vec4(aPos, 1.0));
	FragPosLightSpace = lightSpaceMatrix * vec4(Position, 1.0);
}

