#define _CRT_SECURE_NO_WARNINGS // necessary to get the react physics 3d library to compile

#include "Sphere.h"
#include <reactphysics3d/reactphysics3d.h> 

Sphere::Sphere(reactphysics3d::PhysicsWorld* world, reactphysics3d::PhysicsCommon* physicsCommon, float x, float y, float z, float r) : 
	x(x), y(y), z(z), r(r) {
	
	reactphysics3d::Vector3 position(x, y, z);
	reactphysics3d::Quaternion orientation = reactphysics3d::Quaternion::identity();
	reactphysics3d::Transform transform(position, orientation);

	this->prevTransform = transform;

	this->rigidBody = world->createRigidBody(transform);
	this->rigidBody->setType(reactphysics3d::BodyType::DYNAMIC);
	this->rigidBody->setMass(r * r * r);

	reactphysics3d::SphereShape* sphereShape = physicsCommon->createSphereShape(r);
	reactphysics3d::Transform shapeTransform = reactphysics3d::Transform::identity();

	this->collider = this->rigidBody->addCollider(sphereShape, shapeTransform);

	reactphysics3d::Material& material = this->collider->getMaterial();
}

void Sphere::update(float interpolationFactor) {
	reactphysics3d::Transform currTransform = this->rigidBody->getTransform();

	reactphysics3d::Transform interpolatedTransform = reactphysics3d::Transform::interpolateTransforms(
		prevTransform, currTransform, interpolationFactor);

	this->prevTransform = currTransform;

	this->x = currTransform.getPosition().x;
	this->y = currTransform.getPosition().y;
	this->z = currTransform.getPosition().z;
}

void Sphere::destroy(reactphysics3d::PhysicsWorld* world) {
	world->destroyRigidBody(this->rigidBody);
}

Sphere::~Sphere() {

}