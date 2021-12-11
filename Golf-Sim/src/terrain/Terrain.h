#pragma once

#include <GLCore.h>
#include <vector>
#include <memory>

#include "util/VertexBuffer.h"
#include "util/VertexArray.h"
#include "util/Shader.h"
#include "util/PerspectiveCamera.h"
#include "lights/Lights.h"

class Terrain {
public:
	Terrain(glm::vec3 position, int numHorizontal, int numVertical, float mapWidth, float mapHeight);
	void generateModel(float noiseFreq, float noiseAmp);
	void freeModel();

	void update(GLCore::Timestep ts);
	void render(PerspectiveCamera& camera, lights::LightScene& lightScene);

	void imGuiRender();

private:
	int numRows;
	int numCols;
	float mapWidth;
	float mapHeight;

	glm::vec3 position;

	std::vector<float> heightMap;
	std::vector<float> vertices;

	std::unique_ptr<VertexArray> vertexArray;
	std::unique_ptr<VertexBuffer> vertexBuffer;

	Shader shader;

	std::pair<float, float> getXZ(int index);
	void addVertex(int index, std::vector<float>& norm);
	std::vector<float> getNormal(int i1, int i2, int i3);
};

