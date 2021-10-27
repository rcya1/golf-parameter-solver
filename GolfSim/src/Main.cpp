#define _CRT_SECURE_NO_WARNINGS // necessary to get the react physics 3d library to compile

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
#include "sphere/Sphere.h"
#include "sphere/SphereModel.h"
#include "terrain/Terrain.h"

#include <reactphysics3d/reactphysics3d.h> 

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void process_inputs(GLFWwindow* window);	

int windowWidth = 1600, windowHeight = 1200;

Camera camera = Camera(glm::vec3(0, 10.0, 5.0), 3, 50, 5, -90, -45, 45);
float lastTime = 0.0f;
float deltaTime = 0.0f;
float timeSinceFPSUpdate = 0.0f;
int numUpdatesSinceFPSUpdate = 0;
float accumulatedTime = 0.0f; // for physics calculation
float desiredTimeStep = 1.0 / 60.0f; // for physics calculations

float terrainNoiseFreq = 10.0f;
float terrainNoiseAmp = 5.0f;

float sphereRadius = 0.5;

Terrain* terrain;

reactphysics3d::PhysicsCommon physicsCommon;
reactphysics3d::PhysicsWorld* world;

bool steppingMode = true;

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

	world = physicsCommon.createPhysicsWorld();

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// create shaders
	Shader sphereShader("src/shaders/LightingVertexShader.vert", "src/shaders/LightingFragmentShader.frag");
	Shader lightShader("src/shaders/BasicVertexShader.vert", "src/shaders/BasicFragmentShader.frag");
	Shader visualizeNormalsShader("src/shaders/visualize-normals/VisualizeNormalsVertexShader.vert",
		"src/shaders/visualize-normals/VisualizeNormalsFragmentShader.frag",
		"src/shaders/visualize-normals/VisualizeNormalsGeometryShader.geom");

	// do not change this because the sphere class will scale it when rendering
	VertexArray* sphereVertexArray = sphere::generateSphereModel(1.0);

	float cubeVertices[] = {
		// positions          // normals         
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, 

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
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	};

	VertexArray cubeVertexArray;
	cubeVertexArray.bind();
	VertexBuffer cubeVertexBuffer(216 * sizeof(float), cubeVertices, 6 * sizeof(float), GL_STATIC_DRAW);
	cubeVertexBuffer.setVertexAttribute(0, 3, GL_FLOAT, 0); // position
	cubeVertexBuffer.setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float)); // normal
	cubeVertexArray.unbind();

	terrain = new Terrain(world, &physicsCommon, 10, 10, 10.0f, 10.0f, terrainNoiseFreq, terrainNoiseAmp);

	glEnable(GL_DEPTH_TEST);

	/*int numSpheres = 10;
	glm::vec3 spherePositions[] = {
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
	};*/

	int numSpheres = 4;
	glm::vec3 spherePositions[] = {
		glm::vec3(-2.5f,  10.0f,  2.5f),
		glm::vec3(-2.5f,  10.0f,  -2.5f),
		glm::vec3(2.5f,  10.0f,  2.5f),
		glm::vec3(2.5f,  10.0f,  -2.5f),
	};
	std::vector<Sphere*> spheres;
	for (int i = 0; i < numSpheres; i++) {
		spheres.push_back(new Sphere(world, &physicsCommon, spherePositions[i].x,
			spherePositions[i].y, spherePositions[i].z, sphereRadius));
	}
	
	int numPointLights = 8;
	glm::vec3 pointLightPositions[] = {
		glm::vec3( 25.0f,  25.0f, -25.0f),
		glm::vec3( 25.0f,  25.0f,  25.0f),
		glm::vec3(-25.0f,  25.0f, -25.0f),
		glm::vec3(-25.0f,  25.0f,  25.0f),
		glm::vec3( 25.0f, -25.0f, -25.0f),
		glm::vec3( 25.0f, -25.0f,  25.0f),
		glm::vec3(-25.0f, -25.0f, -25.0f),
		glm::vec3(-25.0f, -25.0f,  25.0f)
	};

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		timeSinceFPSUpdate += deltaTime;
		numUpdatesSinceFPSUpdate++;

		if(timeSinceFPSUpdate >= 2) {
			std::cout << (numUpdatesSinceFPSUpdate / timeSinceFPSUpdate) << std::endl;
			timeSinceFPSUpdate = 0;
			numUpdatesSinceFPSUpdate = 0;
		}

		process_inputs(window);

		accumulatedTime += deltaTime;
		while (accumulatedTime >= desiredTimeStep) {
			accumulatedTime -= desiredTimeStep;
			world->update(desiredTimeStep);
		}

		for (Sphere* sphere : spheres) {
			sphere->update(accumulatedTime / desiredTimeStep);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		sphereShader.activate();
		sphereShader.setVec3f("material.ambient", 0.0f, 0.0f, 0.0f);
		sphereShader.setVec3f("material.diffuse", 0.1f, 0.35f, 0.1f);
		sphereShader.setVec3f("material.specular", 0.25f, 0.25f, 0.25f);
		sphereShader.setFloat("material.shininess", 2);
		sphereShader.setVec3f("viewPos", camera.getPos().x, camera.getPos().y, camera.getPos().z);

		camera.calculateDirection();
		float *cameraViewMatrix = camera.getViewMatrix();
		float* cameraProjectionMatrix = camera.getProjectionMatrix((float)windowWidth / windowHeight);

		sphereShader.setMat4f("view", false, cameraViewMatrix);
		sphereShader.setMat4f("projection", false, cameraProjectionMatrix);

		sphereShader.setInt("numPointLights", numPointLights);
		for (int i = 0; i < numPointLights; i++) {
			std::string prefix = std::string("pointLights[") + std::to_string(i) + std::string("]");
			sphereShader.setVec3f((prefix + ".position").c_str(), pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
			sphereShader.setVec3f((prefix + ".ambient").c_str(), 0.05f, 0.05f, 0.05f);
			sphereShader.setVec3f((prefix + ".diffuse").c_str(), 1.0f, 1.0f, 1.0f);
			sphereShader.setVec3f((prefix + ".specular").c_str(), 1.0f, 1.0f, 1.0f);
			sphereShader.setFloat((prefix + ".constant").c_str(), 1.0f);
			sphereShader.setFloat((prefix + ".linear").c_str(), 0.022);
			sphereShader.setFloat((prefix + ".quadratic").c_str(), 0.0019);
		}

		sphereShader.setInt("numDirLights", 1);
		sphereShader.setVec3f("dirLights[0].direction", 0.0f, -1.0f, 0.0f);
		sphereShader.setVec3f("dirLights[0].ambient", 1.0f, 1.0f, 1.0f);
		sphereShader.setVec3f("dirLights[0].diffuse", 1.0f, 1.0f, 1.0f);
		sphereShader.setVec3f("dirLights[0].specular", 1.0f, 1.0f, 1.0f);

		sphereVertexArray->bind();

		for (Sphere *sphere : spheres) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(sphere->x, sphere->y, sphere->z));
			model = glm::scale(model, glm::vec3(sphere->r, sphere->r, sphere->r));
			sphereShader.setMat4f("model", false, glm::value_ptr(model));

			glDrawElements(GL_TRIANGLES, sphere::indexData.size(), GL_UNSIGNED_INT, (void*) 0);
		}

		glm::mat4 terrainModel = glm::mat4(1.0f);
		terrainModel = glm::translate(terrainModel, glm::vec3(0, 0, 0));
		sphereShader.setMat4f("model", false, glm::value_ptr(terrainModel));
		terrain->render();

		// Visualizing Normals Code
		/*
		visualizeNormalsShader.activate();
		visualizeNormalsShader.setMat4f("projection", false, camera.getProjectionMatrix((float)windowWidth / windowHeight));
		visualizeNormalsShader.setMat4f("view", false, camera.getViewMatrix());
		visualizeNormalsShader.setMat4f("model", false, glm::value_ptr(terrainModel));
		terrain->render();

		sphereVertexArray->bind();
		for (int i = 0; i < 10; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, spherePositions[i]);
			visualizeNormalsShader.setMat4f("model", false, glm::value_ptr(model));

			glDrawElements(GL_TRIANGLES, sphere::indexData.size(), GL_UNSIGNED_INT, (void*)0);
		}
		*/

		lightShader.activate();
		lightShader.setMat4f("view", false, cameraViewMatrix);
		lightShader.setMat4f("projection", false, cameraProjectionMatrix);
		lightShader.setVec3f("vertexColor", 1.0f, 1.0f, 1.0f);

		cubeVertexArray.bind();

		for (int i = 0; i < 8; i++) {
			glm::mat4 lightModel = glm::mat4(1.0f);
			lightModel = glm::translate(lightModel, pointLightPositions[i]);
			lightModel = glm::scale(lightModel, glm::vec3(0.4f));
			lightShader.setMat4f("model", false, glm::value_ptr(lightModel));

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	for (Sphere* sphere : spheres) {
		sphere->destroy(world);
	}

	terrain->destroy(world);

	physicsCommon.destroyPhysicsWorld(world);

	sphereShader.free();
	sphere::freeSphereModel();

	lightShader.free();
	cubeVertexBuffer.free();
	cubeVertexArray.free();

	delete terrain;

	glfwTerminate();

	return 0;
}

void process_inputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		// camera.zoomIn(deltaTime);
		terrainNoiseFreq += 1.0 * deltaTime;
		std::cout << terrainNoiseFreq << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		// camera.zoomOut(deltaTime);
		terrainNoiseFreq -= 1.0 * deltaTime;
		std::cout << terrainNoiseFreq << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_ENTER)) {
		terrain->destroy(world);
		delete terrain;
		terrain = new Terrain(world, &physicsCommon, 10, 10, 10.0f, 10.0f, terrainNoiseFreq, terrainNoiseAmp);
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