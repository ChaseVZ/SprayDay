#include <iostream>
#include "Enemy.h"
#include "GameManager.h"

Enemy::Enemy(vec3 position, vec3 velocity, float boundingRadius) {
	pos = position;
	vel = velocity;
	boRad = boundingRadius;
	exploding = false;
	explodeFrame = 0;
	scale = 1.0;
};

void Enemy::move(Player p) {
	if (!collide(pos + vel, p))
	{
		pos += vel;
	}
}
bool Enemy::collide(vec3 nextPos, Player p) {
	if (nextPos.x + boRad > 125 || nextPos.x - boRad < -125)
	{
		vel = vec3(-vel.x, vel.y, vel.z);
		return true;
	}
	if (nextPos.z + boRad> 125 || nextPos.z - boRad < -125)
	{
		vel = vec3(vel.x, vel.y, -vel.z);
		return true;
	}

	if (sqrtf(pow((nextPos.x - p.pos.x), 2) + pow((nextPos.z - p.pos.z), 2)) < boRad + p.boRad) 
	{
		explode();
		return true;
	}
	return false;
}


void Enemy::explode() {
	exploding = true;

	GameManager* gm = gm->GetInstance();
	gm->setCollision(true);
}
