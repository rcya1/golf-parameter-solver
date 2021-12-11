#include "PerspectiveCameraController.h"

#include "GLCore/Core/Input.h"
#include "GLCore/Core/KeyCodes.h"

PerspectiveCameraController::PerspectiveCameraController(glm::vec3& cameraPos, float yaw, float pitch, float fov, 
	                                                       float aspectRatio, float moveSpeed, float zoomSpeed, 
	                                                       float panSensitivity) : 
	camera(cameraPos, yaw, pitch, fov, aspectRatio), moveSpeed(moveSpeed), zoomSpeed(zoomSpeed), panSensitivity(panSensitivity)
{

}

void PerspectiveCameraController::update(GLCore::Timestep ts)
{
	glm::vec3 cameraPos = camera.getPos();
	glm::vec3 cameraFront = camera.getFront();
	float yaw = camera.getYaw();
	float pitch = camera.getPitch();

	float deltaTime = ts.GetSeconds();

	if (GLCore::Input::IsKeyPressed(HZ_KEY_W))
	{
		cameraPos += glm::vec3(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw))) * deltaTime * moveSpeed;
	}
	else if (GLCore::Input::IsKeyPressed(HZ_KEY_S))
	{
		cameraPos -= glm::vec3(cos(glm::radians(yaw)), 0, sin(glm::radians(yaw))) * deltaTime * moveSpeed;
	}

	if (GLCore::Input::IsKeyPressed(HZ_KEY_A))
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, glm::vec3(0.0, 1.0, 0.0))) * deltaTime * moveSpeed;
	}
	else if (GLCore::Input::IsKeyPressed(HZ_KEY_D))
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, glm::vec3(0.0, 1.0, 0.0))) * deltaTime * moveSpeed;
	}

	if (GLCore::Input::IsKeyPressed(HZ_KEY_SPACE))
	{
		cameraPos += glm::vec3(0, 1, 0) * deltaTime * moveSpeed;
	}
	else if (GLCore::Input::IsKeyPressed(HZ_KEY_LEFT_SHIFT))
	{
		cameraPos -= glm::vec3(0, 1, 0) * deltaTime * moveSpeed;
	}

	camera.setPos(cameraPos);
}

void PerspectiveCameraController::OnEvent(GLCore::Event& e)
{
	GLCore::EventDispatcher dispatcher(e);
	dispatcher.Dispatch<GLCore::MouseScrolledEvent>(GLCORE_BIND_EVENT_FN(PerspectiveCameraController::OnMouseScrolled));
	dispatcher.Dispatch<GLCore::WindowResizeEvent>(GLCORE_BIND_EVENT_FN(PerspectiveCameraController::OnWindowResized));
	dispatcher.Dispatch<GLCore::MouseMovedEvent>(GLCORE_BIND_EVENT_FN(PerspectiveCameraController::OnMouseMoved));
}

bool PerspectiveCameraController::OnMouseScrolled(GLCore::MouseScrolledEvent& e)
{
	camera.setFOV(camera.getFOV() + e.GetYOffset() * zoomSpeed);
	return false;
}

bool PerspectiveCameraController::OnWindowResized(GLCore::WindowResizeEvent& e)
{
	float aspectRatio = static_cast<float>(e.GetWidth()) / e.GetHeight();
	camera.setAspectRatio(aspectRatio);
	return false;
}

bool PerspectiveCameraController::OnMouseMoved(GLCore::MouseMovedEvent& e)
{
	float mouseX = e.GetX();
	float mouseY = e.GetY();

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

	float yaw = camera.getYaw() + xOffset;
	float pitch = camera.getPitch() + yOffset;

	if (pitch > 89.9f)
		pitch = 89.9f;
	if (pitch < -89.9f)
		pitch = -89.9f;

	camera.setRotation(yaw, pitch);
	return false;
}
