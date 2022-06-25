#pragma once

#include <glm/glm.hpp>

/*
RED = 0xff0000,
GREEN = 0x00ff00,
BLUE = 0x0000ff,
BLACK = 0x000000,
WHITE = 0xffffff,
YELLOW = 0xffff00,
MAGENTA = 0xff00ff,
CYAN = 0x00ffff,
*/
inline glm::vec3 convertColor(int color) {
	return glm::vec3((color / (256 * 256)) % 256, (color / 256) % 256, color % 256);
}