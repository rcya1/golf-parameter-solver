#pragma once

#include "util/Shader.h"

#include <glm/glm.hpp>
#include <vector>

namespace lights {

	struct PointLight {
		glm::vec3 position;

		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		float constant;
		float linear;
		float quadratic;
	};

	struct DirLight {
		glm::vec3 direction;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
	};

	struct LightScene {
		std::vector<PointLight> pointLights;
		std::vector<DirLight> dirLights;
	};

	void setLightScene(Shader& shader, LightScene& lightScene);
	PointLight createBasicPointLight(glm::vec3 position);
	DirLight createBasicDirLight(glm::vec3 direction);
};
