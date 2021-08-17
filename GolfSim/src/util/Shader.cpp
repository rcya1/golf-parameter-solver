#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <string>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	std::string vertexCode;
	std::string fragmentCode;

	std::ifstream vertexStream(vertexPath);
	std::ifstream fragmentStream(fragmentPath);

	std::string currLine;
	while (std::getline(vertexStream, currLine)) {
		vertexCode += currLine;
		vertexCode += '\n';
	}
	vertexCode += '\0';
	vertexStream.close();

	while (std::getline(fragmentStream, currLine)) {
		fragmentCode += currLine;
		fragmentCode += '\n';
	}
	fragmentCode += '\0';
	fragmentStream.close();

	const char* vertexCodeC = vertexCode.c_str();
	const char* fragCodeC = fragmentCode.c_str();

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexCodeC, NULL);
	glCompileShader(vertexShader);
	int vertexShaderSuccess;
	char vertexShaderInfoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexShaderSuccess);
	if (!vertexShaderSuccess) {
		glGetShaderInfoLog(vertexShader, 512, NULL, vertexShaderInfoLog);
		std::cout << "Vertex shader from " + std::string(vertexPath) + " failed to compile!\n" << vertexShaderInfoLog << std::endl;
	}
	else {
		std::cout << "Compiled vertex shader from " + std::string(vertexPath) + "!" << std::endl;
	}

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragCodeC, NULL);
	glCompileShader(fragmentShader);
	int fragmentShaderSuccess;
	char fragmentShaderInfoLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentShaderSuccess);
	if (!fragmentShaderSuccess) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, fragmentShaderInfoLog);
		std::cout << "Fragment shader from " + std::string(fragmentPath) + " failed to compile!\n" << fragmentShaderInfoLog << std::endl;
	}
	else {
		std::cout << "Compiled fragment shader from " + std::string(fragmentPath) + "!" << std::endl;
	}

	this->id = glCreateProgram();
	glAttachShader(this->id, vertexShader);
	glAttachShader(this->id, fragmentShader);
	glLinkProgram(this->id);
	int shaderProgramSuccess;
	char shaderProgramInfoLog[512];
	glGetProgramiv(this->id, GL_LINK_STATUS, &shaderProgramSuccess);
	if (!shaderProgramSuccess) {
		glGetProgramInfoLog(this->id, 512, NULL, shaderProgramInfoLog);
		std::cout << "Shader program failed to link!\n" << shaderProgramInfoLog << std::endl;
	}
	else {
		std::cout << "Linked shader program!" << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::free() {
	glDeleteProgram(this->id);
}

void Shader::activate() const {
	glUseProgram(this->id);
}

void Shader::setBool(const char* name, bool value) const {
	int location = glGetUniformLocation(this->id, name);
	glUniform1i(location, value);
}

void Shader::setInt(const char* name, int value) const {
	int location = glGetUniformLocation(this->id, name);
	glUniform1i(location, value);
}

void Shader::setFloat(const char* name, float value) const {
	int location = glGetUniformLocation(this->id, name);
	glUniform1f(location, value);
}

void Shader::setVec3f(const char* name, float value1, float value2, float value3) {
	int location = glGetUniformLocation(this->id, name);
	glUniform3f(location, value1, value2, value3);
}

void Shader::setVec4f(const char* name, float value1, float value2, float value3, float value4) {
	int location = glGetUniformLocation(this->id, name);
	glUniform4f(location, value1, value2, value3, value4);
}

void Shader::setMat3f(const char* name, bool transpose, float* value) {
	int location = glGetUniformLocation(this->id, name);
	glUniformMatrix3fv(location, 1, transpose, value);
}

void Shader::setMat4f(const char* name, bool transpose, float* value) {
	int location = glGetUniformLocation(this->id, name);
	glUniformMatrix4fv(location, 1, transpose, value);
}