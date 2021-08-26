#pragma once

#include <vector>
#include "../util/VertexBuffer.h"
#include "../util/VertexArray.h"
#include "../util/IndexBuffer.h"

class Terrain {
private:
	int numHorizontal;
	int numVertical;
	
	std::vector<float> heightMap;
	std::vector<float> vertices;

	VertexArray *vertexArray;
	VertexBuffer *vertexBuffer;

	std::pair<float, float> getXZ(int index);
	void addVertex(int index, std::vector<float>& norm);
	std::vector<float> getNormal(int i1, int i2, int i3);

public:
	Terrain(int numHorizontal, int numVertical, float noiseFreq, float noiseAmp);
	~Terrain();
	void render();
	void debug();
};

