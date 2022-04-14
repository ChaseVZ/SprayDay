#include "Enemy.h"

Enemy::Enemy(vec3 position, vec3 velocity, float boundingRadius) {
	pos = position;
	vel = velocity;
	boRad = boundingRadius;
};
void Enemy::move() {
	pos += vel;
}
void collide() {

}
void explode() {
}
