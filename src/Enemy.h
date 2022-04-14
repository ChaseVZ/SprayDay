#ifndef ENEMY_H
#define ENEMY_H
#include <glm/gtc/type_ptr.hpp>
using namespace glm;
class Enemy
{
public:
	float boRad;
	vec3 pos;
	vec3 vel;
	void move();
	void collide();
	void explode();
	Enemy(vec3 position, vec3 velocity, float boundingRadius);
};

#endif