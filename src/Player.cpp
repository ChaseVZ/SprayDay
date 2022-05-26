#include <iostream>
#include "Player.h"
#include "GameManager.h"
#include "systems/CollisionSystem.h"
#include <map>
#include <vector>
#include <list>
#include <iostream>

using namespace std;
using namespace glm;

/* HELPER INFO */
float speeds[2] = { 22, 10 };  // from 0.65 and 2.0
vec3 up = vec3(0, 1, 0);

float interpolate(float x1, float x2, float y1, float y2, float x)
{
	float res =  y1 + (x - x1) * ((y2 - y1) / (x2 - x1));
	if (x > x1)
		return y1;
	if (x < x2)
		return y2;
	return res;
}


quat RotationBetweenVectors(vec3 start, vec3 dest) {
	start = normalize(start);
	dest = normalize(dest);

	float cosTheta = dot(start, dest);
	vec3 rotationAxis;

	rotationAxis = cross(start, dest);

	float s = sqrt((1 + cosTheta) * 2);
	float invs = 1 / s;

	return quat(
		s * 0.5f,
		rotationAxis.x * invs,
		rotationAxis.y * invs,
		rotationAxis.z * invs
	);

}

/* PLAYER STUFF */
Player::Player()
{
	localGround = 0;
	w = a = s = d = jumping = false;
	vec3 pos_default = vec3(0, 0, 0);
	pos = pos_default;
	mvm_type = 1;
	boRad = 2;
}

// returns the index of the shape that they player is in (for debugging purposes)
void Player::checkCollision()
{
	//GameManager* gm = GameManager::GetInstance();
	//if (gm->checkCollide(nextPos, 2)) {  }
	//CollisionSys::checkCollisions(nextPos);
	//else { pos = nextPos; }
}

vec3 Player::approxIntegrateAccel() {
	if (jumping || falling) {
		return vec3(0.5) * (acc)*vec3(lastFrametime) * vec3(lastFrametime);
	}
	else
		return vec3(0);
}

vec3 Player::calcNextPos(vec3 lookAt, bool goCamera, float frametime, bool *isMovingForward)
{
	vec3 tempw;
	vec3 temps;
	vec3 tempa;
	vec3 tempd;
	vec3 tempv;

	vec3 lookAtXZ = normalize(vec3(lookAt.x, 0, lookAt.z));

	if (w) 
		tempw = speeds[mvm_type] * lookAtXZ;
	else
		tempw = vec3(0.0);
	if (s)
		temps = -(speeds[mvm_type] * lookAtXZ);
	else
		temps = vec3(0.0);
	if (a)
		tempa = -(speeds[mvm_type] * normalize(glm::cross(lookAtXZ, up)));
	else
		tempa = vec3(0.0);
	if (d)
		tempd = speeds[mvm_type] * normalize(glm::cross(lookAtXZ, up));
	else
		tempd = vec3(0.0);

	// velocity = sum of WASD movements, but do not affect y direction
	tempv = tempw + temps + tempa + tempd;

	vec3 nortempv = (normalize(tempv));
	if (tempv == vec3(0.0)){ //not moving
		nortempv = vec3(0.0);
	}
		
	vel.x = nortempv.x;
	vel.z = nortempv.z;

	if (mvm_type == 1) { //walking
		vel.x = 10.0f * vel.x;
		vel.z = 10.0f * vel.z;
	}
	else {				 //running
		vel.x = 20.0f * vel.x;
		vel.z = 20.0f * vel.z;
	}

	// Only involve gravity when player is jumping or falling
	if (jumping || falling) {
		vel = vel + acc * lastTime;

		// Only have jump speed affect velocity @ start of jump
		if (lastTime == 0 && !falling)
			vel.y = jumpSpeed.y;

		// Stop gravity once player has reached localGround (located in collisions())
		// Otherwise, player is still in the air so keep calculating time 
		//cout << pos.y << " " << lastTime << " " << localGround << endl;
		if (pos.y <= localGround && lastTime != 0) {
			pos.y = localGround;
			vel.y = 0;
			lastTime = 0;
			jumping = false;
			falling = false;
		}
		else
			lastTime = lastTime + frametime;
	}
	
	//vec3 temp = vel * colDir;
	//cout << " updating vel: " << vel.x << " " << vel.y << " " << vel.z << " to: " << temp.x << " " << temp.y << " " << temp.z << endl;
	nextPos = pos + (vel) * frametime + approxIntegrateAccel();
	lastFrametime = frametime;

	//cout << std::boolalpha;
	//cout << "jumping: " << jumping << " falling: " << falling << " localGround: " << localGround << " next y: " << nextPos.y << " " << lastTime << endl;

	// Cap position (otherwise player sometimes goes into ground for a sec at the end of a jump)
	if (nextPos.y < localGround) { nextPos.y = localGround; } //cout << "MOVING DOWN TO " << nextPos.y << endl; }
	if (nextPos.y > localGround && !jumping) { falling = true; } //cout << "FALLING NOW " << endl; }

	return vel;
}

void printVec(vec3 v) {
	cout << v.x << " " << v.y << " " << v.z << endl;
}

// 
void Player::updatePos(vec3 dirMask, bool isCollide, shared_ptr<CollisionSys> cs) {
	//cout << "dirMask: " << dirMask.x << " " << dirMask.y << " " << dirMask.z << endl;
	colDir = dirMask;

	vec3 maskedNextPos = pos + ((vel * lastFrametime) * colDir + approxIntegrateAccel());
	if (maskedNextPos.y < localGround) { maskedNextPos.y = localGround; }

	if (!cs->isCollisionPublic(maskedNextPos)) {
		pos = maskedNextPos;
	}

}


