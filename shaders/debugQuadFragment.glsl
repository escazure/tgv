#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
float near_plane = 1.0;
float far_plane = 20.0;

void main() {             
    float depthValue = texture(depthMap, TexCoords).r;
    
    // Output depth directly as a grayscale color
    FragColor = vec4(vec3(depthValue), 1.0);
}
