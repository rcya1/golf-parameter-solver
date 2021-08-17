#version 330 core

out vec4 FragColor;

uniform vec3 vertexColor;
uniform float brightness;

void main() {
	FragColor = vec4(vertexColor.x, vertexColor.y, vertexColor.z, 1.0);
}