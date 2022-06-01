#ifndef ENEMY_H
#define ENEMY_H

#include <glm/gtc/type_ptr.hpp>
#include "Astar.h"

using namespace glm;

struct Enemy {
	float boRad;
	vec3 vel;
	vec3 nextTile;
	bool exploding;
	int explodeFrame;
	float baseSpeed;
	PathingT pathingType;
	vec3 prevVel = vec3(1, 0, 0);
};

#endif