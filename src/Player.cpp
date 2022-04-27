#include <iostream>
#include "Player.h"
#include "GameManager.h"
#include <map>
#include <vector>
#include <list>
#include <iostream>


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
	GameManager* gm = GameManager::GetInstance();
	if (gm->checkCollide(nextPos, 2)) {  }
	else { pos = nextPos; }
}

void Player::updatePos(vec3 lookAt, bool goCamera, float frametime)
{
	vec3 tempw;
	vec3 temps;
	vec3 tempa;
	vec3 tempd;
	vec3 tempv;

	vec3 moveDir = normalize(vec3(lookAt.x, 0, lookAt.z));

	if (!goCamera) {
		if (w) 
			tempw = speeds[mvm_type] * moveDir;
		if (!w)
			tempw = vec3(0.0);
		if (s)
			temps = -(speeds[mvm_type] * moveDir);
		if (!s)
			temps = vec3(0.0);
		if (a)
			tempa = -(speeds[mvm_type] * normalize(glm::cross(moveDir, up)));
		if (!a)
			tempa = vec3(0.0);
		if (d)
			tempd = speeds[mvm_type] * normalize(glm::cross(moveDir, up));
		if (!d)
			tempd = vec3(0.0);

		// velocity = sum of WASD movements, but do not effect y direction
		tempv = tempw + temps + tempa + tempd;

		vec3 nortempv = abs(normalize(tempv));
		if (tempv.x == 0.0 && tempv.y == 0.0 && tempv.z == 0.0){
			nortempv = vec3(0.0);
		}
		else {
			tempv = nortempv*normalize(tempv);
		}

		// cap speed to movement speed type
		vel.x = std::min(tempv.x, speeds[mvm_type]);
		vel.z = std::min(tempv.z, speeds[mvm_type]);

		if (mvm_type == 1) { //walking
			vel.x = 10.0f * vel.x;
			vel.z = 10.0f * vel.z;
		}
		else {				 //running
			vel.x = 20.0f * vel.x;
			vel.z = 20.0f * vel.z;
		}
		//cout << "Velocity: " << vel.x << " " << vel.z << "\n";
		//cout << "Total Velocity: " << sqrt(vel.x*vel.x) + sqrt(vel.z*vel.z)<< "\n\n";

		// Only involve gravity when player is jumping
		if (jumping) {
			cout << "vel.y before: " << vel.y << "\n";
			vel = vel + acc * lastTime;
			cout << "vel.y after : " << vel.y << "\n";

			// Only have jump speed affect velocity @ start of jump
			if (lastTime == 0)
				vel += jumpSpeed;

			// Stop gravity once player has reached localGround (located in collisions())
			// Otherwise, player is still in the air so keep calculating time 
			if (pos.y <= localGround && lastTime != 0) {
				pos.y = localGround;
				vel.y = 0;
				lastTime = 0;
				jumping = false;
			}
			else
				lastTime = lastTime + frametime;
			cout << "vel.y: " << vel.y << "\n\n";
		}

	

		nextPos = pos + vel*frametime;
		// if (nextPos.y > localGround) {
		// 	cout << "in the air!!" << "\n";
		// }
		checkCollision();
		//pos = nextPos;

		// << pos.x << " " << pos.z << endl;

		// Cap position (otherwise player sometimes goes into ground for a sec at the end of a jump)
		if (pos.y < localGround) {pos.y = localGround;}
		if (!jumping && pos.y > localGround) { pos.y = localGround; cout << "in the air!!" << "\n";}
 	}
}


