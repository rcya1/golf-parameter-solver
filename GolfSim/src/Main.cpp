#include <iostream>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "util/Shader.h"
#include "util/Camera.h"
#include "util/IndexBuffer.h"
#include "util/VertexBuffer.h"
#include "util/VertexArray.h"
#include "sphere/SphereModel.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void process_inputs(GLFWwindow* window);	

int windowWidth = 1600, windowHeight = 1200;

Camera camera = Camera(glm::vec3(0, 0, 3.0), 3, 50, 5, -90, 0, 45);
float lastTime = 0.0f;
float deltaTime = 0.0f;

int main() {
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(windowWidth, windowHeight, "Hello World", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	// create shader and set up texture mappings
	Shader shader("src/shaders/LightingVertexShader.vert", "src/shaders/LightingFragmentShader.frag");
	shader.activate();

	VertexArray* sphereVertexArray = sphere::generateSphereModel(0.5);

	VertexArray cubeVertexArray;
	cubeVertexArray.bind();

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	VertexBuffer cubeVertexBuffer(216 * sizeof(float), vertices, 6 * sizeof(float), GL_STATIC_DRAW);
	cubeVertexBuffer.setVertexAttribute(0, 3, GL_FLOAT, 0); // position
	cubeVertexBuffer.setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float)); // normals

	glEnable(GL_DEPTH_TEST);

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		process_inputs(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.activate();
		shader.setVec3f("objectColor", 1.0f, 0.5f, 0.31f);
		shader.setFloat("shininess", 32.0f);
		shader.setVec3f("lightColor", 1.0f, 1.0f, 1.0f);
		shader.setVec3f("lightPos", 1.2f, 1.0f, 2.0f);
		shader.setVec3f("viewPos", camera.getPos().x, camera.getPos().y, camera.getPos().z);

		camera.calculateDirection();
		shader.setMat4f("view", false, camera.getViewMatrix());
		shader.setMat4f("projection", false, camera.getProjectionMatrix((float) windowWidth / windowHeight));

		shader.setInt("numPointLights", 4);
		for (int i = 0; i < 4; i++) {
			std::string prefix = std::string("pointLights[") + std::to_string(i) + std::string("]");
			shader.setVec3f((prefix + ".position").c_str(), pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
			shader.setVec3f((prefix + ".ambient").c_str(), 0.05f, 0.05f, 0.05f);
			shader.setVec3f((prefix + ".diffuse").c_str(), 0.8f, 0.8f, 0.8f);
			shader.setVec3f((prefix + ".specular").c_str(), 1.0f, 1.0f, 1.0f);
			shader.setFloat((prefix + ".constant").c_str(), 1.0f);
			shader.setFloat((prefix + ".linear").c_str(), 0.09);
			shader.setFloat((prefix + ".quadratic").c_str(), 0.032);
		}

		shader.setInt("numDirLights", 1);
		shader.setVec3f("dirLights[0].direction", -0.2f, -1.0f, -0.3f);
		shader.setVec3f("dirLights[0].ambient", 0.05f, 0.05f, 0.05f);
		shader.setVec3f("dirLights[0].diffuse", 0.4f, 0.4f, 0.4f);
		shader.setVec3f("dirLights[0].specular", 0.5f, 0.5f, 0.5f);

		// cubeVertexArray.bind();
		sphereVertexArray->bind();

		for (int i = 0; i < 10; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			// float angle = 30.0f * i;
			// model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0 - i, 2 * i, i * i));
			shader.setMat4f("model", false, glm::value_ptr(model));

			shader.setVec3f("vertexColor", i * 0.1, 0.5, 0.5);

			glDrawElements(GL_TRIANGLES, sphere::indexData.size(), GL_UNSIGNED_INT, (void*) 0);
			// glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	shader.free();
	cubeVertexArray.free();
	cubeVertexBuffer.free();

	glfwTerminate();

	return 0;
}

void process_inputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		camera.zoomIn(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		camera.zoomOut(deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.moveForward(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.moveBackward(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.moveLeft(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.moveRight(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.moveDown(deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.moveUp(deltaTime);
	}
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
	camera.updateCamera(xPos, yPos);
	camera.calculateDirection();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);
}