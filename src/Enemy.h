#ifndef ENEMY_H
#define ENEMY_H

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct Enemy {
	float boRad;
	vec3 vel;
	vec3 nextTile;
	bool exploding;
	int explodeFrame;
	float baseSpeed;
};

#endif