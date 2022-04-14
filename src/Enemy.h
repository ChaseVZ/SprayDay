#ifndef ENEMY_H
#define ENEMY_H

#include <glm/gtc/type_ptr.hpp>
#include "Player.h"

using namespace glm;
class Enemy
{
public:
	float boRad;
	vec3 pos;
	vec3 vel;
	bool exploding;
	float scale;

	void move(Player p);
	bool collide(vec3 nextPos, Player p);
	void explode();
	Enemy(vec3 position, vec3 velocity, float boundingRadius);
};

#endif