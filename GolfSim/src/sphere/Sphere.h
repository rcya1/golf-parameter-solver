#pragma once

#include <reactphysics3d/reactphysics3d.h> 

class Sphere {
private:
	reactphysics3d::RigidBody* rigidBody;
	reactphysics3d::Collider* collider;
	reactphysics3d::Transform prevTransform;

public:
	float x, y, z;
	float r;

	Sphere(reactphysics3d::PhysicsWorld *world, reactphysics3d::PhysicsCommon *physicsCommon,
		float x, float y, float z, float r);
	~Sphere();

	void update(float interpolationFactor);
	void destroy(reactphysics3d::PhysicsWorld* world);
};

