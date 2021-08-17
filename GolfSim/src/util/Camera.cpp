#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(glm::vec3 cameraPos, float moveSpeed, float zoomSpeed, float cameraSpeed, float yaw, float pitch, float fov) {
	this->cameraPos = cameraPos;
	this->moveSpeed = moveSpeed;
	this->zoomSpeed = zoomSpeed;
	this->cameraSpeed = cameraSpeed;
	this->yaw = yaw;
	this->pitch = pitch;
	this->fov = fov;

	calculateDirection();
}

Camera::~Camera() {

}

float* Camera::getViewMatrix() {
	this->view = glm::lookAt(cameraPos, this->cameraPos + this->cameraFront, glm::vec3(0.0, 1.0, 0.0));
	return glm::value_ptr(this->view);
}

float* Camera::getProjectionMatrix(float aspectRatio) {
	this->projection = glm::mat4(1.0f);
	this->projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);
	return glm::value_ptr(this->projection);
}

glm::vec3 Camera::getPos() {
	return this->cameraPos;
}

glm::vec3 Camera::getFront() {
	return this->cameraFront;
}

void Camera::calculateDirection() {
	this->cameraFront = glm::vec3(
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
}

void Camera::moveForward(float deltaTime) {
	cameraPos += glm::vec3(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw))) * deltaTime * cameraSpeed;
}

void Camera::moveBackward(float deltaTime) {
	cameraPos -= glm::vec3(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw))) * deltaTime * cameraSpeed;
}

void Camera::moveLeft(float deltaTime) {
	cameraPos -= glm::normalize(glm::cross(cameraFront, glm::vec3(0.0, 1.0, 0.0))) * deltaTime * cameraSpeed;
}

void Camera::moveRight(float deltaTime) {
	cameraPos += glm::normalize(glm::cross(cameraFront, glm::vec3(0.0, 1.0, 0.0))) * deltaTime * cameraSpeed;
}

void Camera::moveUp(float deltaTime) {
	cameraPos += glm::vec3(0, 1, 0) * deltaTime * cameraSpeed;
}

void Camera::moveDown(float deltaTime) {
	cameraPos -= glm::vec3(0, 1, 0) * deltaTime * cameraSpeed;
}

void Camera::zoomIn(float deltaTime) {
	this->fov -= deltaTime * this->zoomSpeed;
}

void Camera::zoomOut(float deltaTime) {
	this->fov += deltaTime * this->zoomSpeed;
}

void Camera::updateCamera(float mouseX, float mouseY) {
	if (firstMouse) {
		lastMouseX = mouseX;
		lastMouseY = mouseY;
		firstMouse = false;
	}

	float xOffset = mouseX - lastMouseX;
	float yOffset = lastMouseY - mouseY;

	lastMouseX = mouseX;
	lastMouseY = mouseY;

	const float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	if (pitch > 89.9f)
		pitch = 89.9f;
	if (pitch < -89.9f)
		pitch = -89.9f;
}
