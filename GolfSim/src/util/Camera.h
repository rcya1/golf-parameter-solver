#pragma once

#include <glm/glm.hpp>

class Camera {
private:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	float yaw;
	float pitch;
	float fov;
	float moveSpeed;
	float zoomSpeed;
	float cameraSpeed;

	glm::mat4 view;
	glm::mat4 projection;

	float lastMouseX;
	float lastMouseY;
	bool firstMouse = true;
public:
	Camera(glm::vec3 cameraPos, float moveSpeed, float zoomSpeed, float cameraSpeed, float yaw, float pitch, float fov);
	~Camera();
	float* getViewMatrix();
	float* getProjectionMatrix(float aspectRatio);
	glm::vec3 getPos();
	glm::vec3 getFront();

	void calculateDirection();
	void moveForward(float deltaTime);
	void moveBackward(float deltaTime);
	void moveLeft(float deltaTime);
	void moveRight(float deltaTime);
	void moveUp(float deltaTime);
	void moveDown(float deltaTime);
	void zoomIn(float deltaTime);
	void zoomOut(float deltaTime);
	void updateCamera(float mouseX, float mouseY);

	float getYaw() {
		return yaw;
	}

	float getPitch() {
		return pitch;
	}

	void setCameraSpeed(float cameraSpeed) {
		this->cameraSpeed = cameraSpeed;
	}

	void setMoveSpeed(float cameraSpeed) {
		this->moveSpeed = moveSpeed;
	}
};
