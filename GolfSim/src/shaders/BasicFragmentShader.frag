#version 330 core

out vec4 FragColor;

in vec3 vertexColor;

uniform float brightness;

void main() {
	FragColor = vec4(vertexColor.x + brightness, vertexColor.y + brightness, vertexColor.z + brightness, 1.0);
}