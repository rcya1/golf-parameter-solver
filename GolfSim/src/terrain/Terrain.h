#pragma once

#include <vector>
#include "../util/VertexBuffer.h"
#include "../util/VertexArray.h"
#include "../util/IndexBuffer.h"

#include <reactphysics3d/reactphysics3d.h> 

class Terrain {
private:
	int numRows;
	int numCols;
	float mapWidth;
	float mapHeight;
	
	std::vector<float> heightMap;
	std::vector<float> vertices;

	VertexArray *vertexArray;
	VertexBuffer *vertexBuffer;

	reactphysics3d::RigidBody* rigidBody;
	reactphysics3d::Collider* collider;

	std::pair<float, float> getXZ(int index);
	void addVertex(int index, std::vector<float>& norm);
	std::vector<float> getNormal(int i1, int i2, int i3);

public:
	Terrain(reactphysics3d::PhysicsWorld* world, reactphysics3d::PhysicsCommon* physicsCommon, 
		int numHorizontal, int numVertical, float mapWidth, float mapHeight, float noiseFreq, float noiseAmp);
	~Terrain();
	void render();
	void destroy(reactphysics3d::PhysicsWorld* world);
	void debug();
};

