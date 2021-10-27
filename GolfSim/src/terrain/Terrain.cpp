#define _CRT_SECURE_NO_WARNINGS // necessary to get the react physics 3d library to compile

#include "Terrain.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <reactphysics3d/reactphysics3d.h> 
#include <vector>
#include "../util/VertexBuffer.h"
#include "../util/VertexArray.h"
#include "../util/IndexBuffer.h"
#include "./PerlinNoise.h"

#include <iostream>

std::pair<float, float> Terrain::getXZ(int index) {
	int j = index / (numCols + 1);
	int i = index % (numCols + 1);

	const float hSpacing = this->mapWidth / numCols;
	const float vSpacing = this->mapHeight / numRows;

	return std::make_pair(i * hSpacing - this->mapWidth / 2, j * vSpacing - this->mapHeight / 2);
}

void Terrain::addVertex(int index, std::vector<float> &norm) {
	std::pair<float, float> xz = getXZ(index);
	float x = xz.first;
	float y = heightMap[index];
	float z = xz.second;

	vertices.push_back(x);
	vertices.push_back(y);
	vertices.push_back(z);

	vertices.push_back(norm[0]);
	vertices.push_back(norm[1]);
	vertices.push_back(norm[2]);
}

// returns the upward facing normal (one with positive y coordinate)
std::vector<float> Terrain::getNormal(int i1, int i2, int i3) {
	std::pair<float, float> xz1 = getXZ(i1);
	std::pair<float, float> xz2 = getXZ(i2);
	std::pair<float, float> xz3 = getXZ(i3);

	glm::vec3 a(xz1.first - xz2.first, heightMap[i1] - heightMap[i2], xz1.second - xz2.second);
	glm::vec3 b(xz3.first - xz2.first, heightMap[i3] - heightMap[i2], xz3.second - xz2.second);

	glm::vec3 c = glm::normalize(glm::cross(a, b));

	if (c.y < 0) c *= -1;

	return std::vector<float>{c.x, c.y, c.z};
}

Terrain::Terrain(reactphysics3d::PhysicsWorld* world, reactphysics3d::PhysicsCommon* physicsCommon, 
		int numRows, int numCols, float mapWidth, float mapHeight, float noiseFreq, float noiseAmp)
	: numRows(numRows), numCols(numCols), mapWidth(mapWidth), mapHeight(mapHeight) {

	heightMap = std::vector<float>((numRows + 1) * (numCols + 1));

	const float hSpacing = this->mapWidth / numCols;
	const float vSpacing = this->mapHeight / numRows;

	noise::initNoise();
	
	for (int j = 0; j <= numCols; j++) {
		for (int i = 0; i <= numRows; i++) {
			float x = i * hSpacing - this->mapWidth / 2;
			float z = j * vSpacing - this->mapHeight / 2;

			//float height = noiseAmp;
			//float height = (x * x + z * z) / 10.0;
			float height = noise::noise(x / noiseFreq, -5, z / noiseFreq) * noiseAmp;

			heightMap[j * (numCols + 1) + i] = height;
		}
	}

	for (int j = 0; j < numCols; j++) {
		for (int i = 0; i < numRows; i++) {

			int topLeft = j * (numCols + 1) + i;
			int topRight = (j + 1) * (numCols + 1) + i;
			int botLeft = j * (numCols + 1) + i + 1;
			int botRight = (j + 1) * (numCols + 1)+ i + 1;

			std::vector<float> n1 = getNormal(topLeft, topRight, botLeft);
			addVertex(topLeft , n1);
			addVertex(topRight, n1);
			addVertex(botLeft , n1);

			std::vector<float> n2 = getNormal(botLeft, topRight, botRight);
			addVertex(botLeft , n2);
			addVertex(topRight, n2);
			addVertex(botRight, n2);
		}
	}

	vertexArray = new VertexArray();
	vertexArray->bind();

	vertexBuffer = new VertexBuffer(vertices.size() * sizeof(float), vertices.data(), 6 * sizeof(float), GL_STATIC_DRAW);
	vertexBuffer->setVertexAttribute(0, 3, GL_FLOAT, 0);
	vertexBuffer->setVertexAttribute(1, 3, GL_FLOAT, 3 * sizeof(float));

	vertexArray->unbind();

	reactphysics3d::Vector3 position(0, 0, 0);
	reactphysics3d::Quaternion orientation = reactphysics3d::Quaternion::identity();
	reactphysics3d::Transform transform(position, orientation);

	this->rigidBody = world->createRigidBody(transform);
	this->rigidBody->setType(reactphysics3d::BodyType::STATIC);
	
	reactphysics3d::HeightFieldShape* heightFieldShape = physicsCommon->createHeightFieldShape(
		numRows + 1, numCols + 1, -noiseAmp, noiseAmp, heightMap.data(), 
		reactphysics3d::HeightFieldShape::HeightDataType::HEIGHT_FLOAT_TYPE);
	reactphysics3d::Transform shapeTransform = reactphysics3d::Transform::identity();

	this->collider = this->rigidBody->addCollider(heightFieldShape, shapeTransform);

	/*reactphysics3d::Vector3 a;
	reactphysics3d::Vector3 b;
	heightFieldShape->getLocalBounds(a, b);
	std::cout << a.x << " : " << a.y << " : " << a.z << std::endl;
	std::cout << b.x << " : " << b.y << " : " << b.z << std::endl;*/
}

void Terrain::render() {
	vertexArray->bind();

	glDrawArrays(GL_TRIANGLES, 0, 2 * 3 * numRows * numCols);
}

void Terrain::debug() {
	int c = 0;
	for (auto x : vertices) {
		std::cout << x << " ";
		c++;
		if (c % 6 == 0) std::cout << std::endl;
	}

	std::cout << std::endl;
}

void Terrain::destroy(reactphysics3d::PhysicsWorld* world) {
	world->destroyRigidBody(this->rigidBody);
}

Terrain::~Terrain() {
	vertexBuffer->free();
	vertexArray->free();

	delete vertexArray;
	delete vertexBuffer;
}
