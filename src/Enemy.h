#ifndef ENEMY_H
#define ENEMY_H

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct Enemy {
	float boRad;
	vec3 vel;
	bool exploding;
	int explodeFrame;
};

#endif