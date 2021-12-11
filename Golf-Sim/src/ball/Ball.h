#pragma once

#include <GLCore.h>

class Ball {
public:
	Ball();
	Ball(float x, float y, float z, float r);

	void update(GLCore::Timestep ts);
	void render();

	void imGuiRender();

private:
	glm::vec3 position;
	float radius;
};

