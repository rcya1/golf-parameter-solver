#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

#define MAX_DIR_LIGHTS 3

out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace[MAX_DIR_LIGHTS];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix[MAX_DIR_LIGHTS];

uniform int numDirLights;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
	for(int i = 0; i < min(numDirLights, MAX_DIR_LIGHTS); i++) {
        FragPosLightSpace[i] = lightSpaceMatrix[i] * vec4(FragPos, 1.0);
	}
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

