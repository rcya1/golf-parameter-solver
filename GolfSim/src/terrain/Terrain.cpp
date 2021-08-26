#include "Terrain.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include "../util/VertexBuffer.h"
#include "../util/VertexArray.h"
#include "../util/IndexBuffer.h"
#include "./PerlinNoise.h"

#include <iostream>

#define MAP_WIDTH 10.0f
#define MAP_HEIGHT 10.0f

std::pair<float, float> Terrain::getXZ(int index) {
	int i = index / numVertical;
	int j = index % numVertical;

	const float hSpacing = MAP_WIDTH / numVertical;
	const float vSpacing = MAP_HEIGHT / numHorizontal;

	return std::make_pair(i * hSpacing, j * vSpacing);
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

Terrain::Terrain(int numHorizontal, int numVertical, float noiseFreq, float noiseAmp) 
	: numHorizontal(numHorizontal), numVertical(numVertical) {
	heightMap = std::vector<float>(numHorizontal * numVertical);

	const float hSpacing = MAP_WIDTH / numVertical;
	const float vSpacing = MAP_HEIGHT / numHorizontal;

	noise::initNoise();
	
	for (int i = 0; i < numHorizontal; i++) {
		for (int j = 0; j < numVertical; j++) {
			float x = i * hSpacing;
			float z = j * vSpacing;

			//float height = ((x-MAP_WIDTH / 2) * (x- MAP_WIDTH / 2) + (z-MAP_HEIGHT / 2) * (z-MAP_HEIGHT / 2)) / 10.0;
			float height = noise::noise(x / noiseFreq, -10, z / noiseFreq) * noiseAmp;

			heightMap[i * numVertical + j] = height;
		}
	}

	for (int i = 0; i < numHorizontal - 1; i++) {
		for (int j = 0; j < numVertical - 1; j++) {
			int topLeft = i * numVertical + j;
			int topRight = (i + 1) * numVertical + j;
			int botLeft = i * numVertical + j + 1;
			int botRight = (i + 1) * numVertical + j + 1;

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
}

void Terrain::render() {
	vertexArray->bind();

	glDrawArrays(GL_TRIANGLES, 0, 2 * 3 * (numHorizontal - 1) * (numVertical - 1));
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

Terrain::~Terrain() {
	vertexBuffer->free();
	vertexArray->free();

	delete vertexArray;
	delete vertexBuffer;
}
