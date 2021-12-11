#include "Ball.h"
#include "BallRenderer.h"

#include <GLCoreUtils.h>

Ball::Ball() : Ball(0.0f, 0.0f, 0.0f, 1.0f) {

}

Ball::Ball(float x, float y, float z, float r) : position(x, y, z), radius(r) {

}

void Ball::update(GLCore::Timestep ts) {

}

void Ball::render() {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position.x, position.y, position.z));
	model = glm::scale(model, glm::vec3(radius, radius, radius));
	BallRenderer::getInstance().add(model);
}

void Ball::imGuiRender() {

}
