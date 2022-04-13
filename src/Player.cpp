#include <iostream>
#include "Player.h"
#include <map>
#include <vector>
#include <list>

using namespace glm;

/* HELPER INFO */
float speeds[2] = { 0.65, 0.2 };
float ep = 0.85f;		// hitbox epsilon
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

bool manualWall(vec3 pos)
{
	if (pos.y > 3.165 && pos.x < -36.1 && pos.z > -43.3 && pos.z < - 6.0)
		return true;
	if (pos.y > 3.165 && pos.z > -34.8 && pos.z < -22.6 && pos.x > -28.2) {
		return true;
	}
	return false;
}


quat RotationBetweenVectors(vec3 start, vec3 dest) {
	start = normalize(start);
	dest = normalize(dest);

	float cosTheta = dot(start, dest);
	vec3 rotationAxis;

	//if (cosTheta < -1 + 0.001f) {
	//	// special case when vectors in opposite directions:
	//	// there is no "ideal" rotation axis
	//	// So guess one; any will do as long as it's perpendicular to start
	//	rotationAxis = cross(vec3(0.0f, 0.0f, 1.0f), start);
	//	if (gtx::norm::length2(rotationAxis) < 0.01) // bad luck, they were parallel, try again!
	//		rotationAxis = cross(vec3(1.0f, 0.0f, 0.0f), start);

	//	rotationAxis = normalize(rotationAxis);
	//	return gtx::quaternion::angleAxis(glm::radians(180.0f), rotationAxis);
	//}

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



/* GUN STUFF*/
Gun::Gun() 
{
	pos = vec3(0, 0, 0);
}

void Gun::update(float frametime, vec3 pos, vec3 lookAt, bool playerShooting, vector<vec3>& enemyPositions)
{
	/* update existing bullets */
	int i = 0;
	vector<int> toDelete;
	for (auto b : bullets) {
		if (b->update(enemyPositions)) {
			toDelete.push_back(i);
		}
		i += 1;
	}

	for (int i : toDelete) {
		bullets.erase(bullets.begin() + i);
	}

	if (playerShooting)
	{
		/* check if new bullet should spawn */
		if (timeSinceLastBullet - frametime > shootThreshold)
		{
			auto b = make_shared<Bullet>(pos, lookAt);
			bullets.push_back(b);
		}
	}

	timeSinceLastBullet += frametime;
}

Bullet::Bullet(vec3 p, vec3 r)
{
	pos = p;
	ray = r;
}

bool Bullet::update(vector<vec3>& enemyPositions)
{
	pos += ray * 2.0f;
	if (pos.z < -60 || pos.z > 76 || pos.x < -120 || pos.x > 10	)
		return true;

	int i = 0;
	bool flag = false;
	for (vec3 target : enemyPositions)
	{
		if (pos.x <= target.x + ep && pos.x >= target.x - ep && pos.y >= target.y && pos.y <= target.y + 2.0f && pos.z <= target.z + ep && pos.z >= target.z - ep) {
			flag = true;
			break;
		}
		i += 1;
	}

	if (flag) {
		enemyPositions.erase(enemyPositions.begin() + i);
	}
	return flag;
}



/* ARROW STUFF */
Arrow::Arrow()
{
	vel = vec3(0, 0, 0);
	pos = vec3(0, 0, 0);
	charges = 0;
}

// holdTime is in seconds
void Arrow::setVelocity(float holdTime, vec3 lookAt)
{
	
	// truncate value and set max value = 2
	int scalar = int(holdTime) + 1;
	scalar = std::min(scalar, 3);
	shot = true;
	instanced = true;
	vel = lookAt * vec3(scalar, scalar, scalar);
	lookAtZ_Store = lookAt.z;
	lookAtX_Store = lookAt.x;

	myQuaternion = RotationBetweenVectors(pos, lookAt);
	//Mat4 rot = glm::lookAt(pos + vel);
	//rotationY = interpolate(1, -1, startRotationY, endRotationY, lookAt.x);
	//rotationY = interpolate(1, -1, startRotationY, endRotationY, lookAt.y);
	//totalTime = 2.0 * vel.y / (-acc.y * 1);  	// this calculation does not work
	//if (lookAt.x == 0 || lookAt.z == 0)
	//	rotationY = 0;
	//else if (abs(lookAt.x + lookAt.z) < 1.4 && abs(lookAt.x + lookAt.z) > 1.0)
	//	rotationY = interpolate(1.4, 1.0, 45, 0, abs(lookAt.x + lookAt.z));
	//else if (abs(lookAt.x + lookAt.z) < 1.0 && abs(lookAt.x + lookAt.z) > 0)
	//	rotationY = interpolate(1.0, 0, 0, -45, abs(lookAt.x + lookAt.z));
	//else 
	//	rotationY = interpolate(1.4, 0, startRotationY, endRotationY, abs(lookAt.x + lookAt.z));
	

	totalTime = 1.6 * scalar * (charges + 1);
}

void Arrow::update(float frameTime, vec3 playerPos, vec3 lookAt, std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls)
{	
	//totalTime = 2;


	/* SET VEL */
	if (shot && !landed) {
		vel = vel + acc * lastTime;
		vec3 nextPos = pos + vel * lastTime;
		//myQuaternion = RotationBetweenVectors(nextPos, lookAt);

		if (lastTime != 0 && collision(floors, walls, nextPos)) {
			if (charges > 0) {
				vel.y = vel.y * -1.0;
				charges -= 1;
			}
			else {
				landed = true;
				vel = vec3(0, 0, 0);
			}
		}
		else
			pos = nextPos;

		rotationX = interpolate(totalTime, 0, -endRotationX * lookAtZ_Store, -startRotationX * lookAtZ_Store, lastTime);
		rotationZ = interpolate(totalTime, 0, -endRotationZ * lookAtX_Store, -startRotationZ * lookAtX_Store, lastTime);
		lastTime = lastTime + frameTime;
	}
	if (!shot && !landed) {
		pos = playerPos + vec3(0.2, 2.5, 0.1);
	}
	if (shot && landed) {
		if (playerPos.x >= pos.x - ep && playerPos.x <= pos.x + ep && playerPos.z >= pos.z - ep && playerPos.z <= pos.z + ep) {
			shot = false;
			landed = false;
			instanced = false;
			pos = playerPos + vec3(0.2, 2.5, 0.1);
			lastTime = 0;
		}
	}


	if (debugMode) {
		cout << "lookAt " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
		cout << "Arrow POS " << pos.x << " " << pos.y << " " << pos.z << endl;
		cout << "Player POS " << playerPos.x << " " << playerPos.y << " " << playerPos.z << endl;
	/*	cout << "Arrow VEL " << vel.x << " " << vel.y << " " << vel.z << endl; */
		cout << "totalTime " << totalTime << endl;
		cout << "currTime " << lastTime << endl;
		cout << "ROTX " << rotationX << endl;
		cout << "ROTZ " << rotationZ << endl;	
		cout << "ROTY " << rotationY << endl << endl;
	}
}

bool Arrow::collision(std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls, vec3 nextPos)
{
	float currShape = -1;
	float localGround = -1;
	for (auto& it : floors)
	{
		// retrieve bb of each shape
		vec3 min = it.second[0];
		vec3 max = it.second[1];

		// Is the character inside this shape?
		if (nextPos.x > min.x && nextPos.x < max.x && nextPos.z > min.z && nextPos.z < max.z) {
			if (!std::count(excludeCollisionFloor.begin(), excludeCollisionFloor.end(), it.first)) {
				//cout << "HIT GROUND " << it.first << endl;
				//cout << max.y << " " << min.y << endl;
				currShape = it.first;
				localGround = max.y;
			}
		}
	}

	// if currShape == -1, means above no floor, means inside a wall
	/* Do some manual ramp calulations */
	if (currShape == -1) {
		//localGround = 99;
		return true;
	}
	else if (currShape == 94)
		localGround = interpolate(-31, -39, 0, 2.112, pos.z);
	else if (currShape == 41)
		localGround = interpolate(-39, -47.8, 2.112, 4.224, pos.z);
	else if (currShape == 612 || currShape == 364)
		localGround = interpolate(6.8, 3.0, -3.168, -4.224, pos.z);
	else if (currShape == 1150 || currShape == 978)
		localGround = interpolate(19.49, 6.99, 0, -3.168, pos.z);
	else if (currShape == 535 || currShape == 528 || currShape == 370)
		localGround = interpolate(-70.4, -76.5, -4.224, -2.112, pos.x);
	else if (currShape == 549 || currShape == 550 || currShape == 548)
		localGround = interpolate(-76.5, -82.9, -2.112, 0, pos.x);
	else if (currShape == 1387)
		localGround = interpolate(-57.8, -61.7, 3.168, 4.224, pos.x);
	else if (currShape == 1119)
		localGround = interpolate(-45, -57.8, 0, 3.168, pos.x);
	else if (currShape == 1415)
		localGround = 4.224;
	else if (currShape == 117)
		localGround = interpolate(-11, -15.5, 0, -1.056, pos.x);
	else if (currShape == 165)
		localGround = interpolate(-15.4, -28, -1.056, -4.224, pos.x);
	else if (currShape == 1358)
		localGround = interpolate(53.8, 49.4, 1.188, 0, pos.z);
	else if (currShape == 1481)
		localGround = interpolate(65.8, 53.8, 4.224, 1.188, pos.z);
	else if (currShape == 1283 || currShape == 1161)
		localGround = 1.056;
	//else
	//	return true;
	 
	//cout << "local " << localGround << endl;

	if (nextPos.y < localGround + 0.1) {
		pos.y = localGround;
		return true;
	}


	for (auto& it : walls)
	{
		vec3 min = it.second[0];
		vec3 max = it.second[1];

		if (nextPos.x > min.x && nextPos.x < max.x && nextPos.z > min.z && nextPos.z < max.z) {
			if (!std::count(excludeCollisionWall.begin(), excludeCollisionWall.end(), it.first)) {
				//cout << "HIT WALL" << it.first << endl;
				return true;
			}
		}
	}
	return false;
}



/* PLAYER STUFF */
Player::Player()
{
	localGround = 0;
	w = a = s = d = jumping = false;
	vec3 pos_default = vec3(0, 0, 0);
	abilityOneInUse = false;
	abilityTwoInUse = false;
	pos = pos_default;
	mvm_type = 1;
}

// returns the index of the shape that they player is in (for debugging purposes)
int Player::collision(std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls)
{
	// lazy way of making a tuple
	std::list<vec2> elevations;

	for (auto& it : floors)
	{
		// retrieve bb of each shape
		vec3 min = it.second[0];
		vec3 max = it.second[1];

		// Is the character inside this shape?
		if (nextPos.x > min.x && nextPos.x < max.x && nextPos.z > min.z && nextPos.z < max.z) {
			if (!std::count(excludeCollisionFloor.begin(), excludeCollisionFloor.end(), it.first))
				elevations.push_back(vec2(it.first, max.y));  // BAD FOR RAMPS
		}
	}

	// if multiple elevations exist at given x,z then determine which elevation is closest to current player.y position
	float closest = 99999;
	float elevation = 999;
	int currShape = 0;
	for (vec2 item : elevations)
	{
		if (abs(item.y - localGround) < abs(closest)) {
			currShape = item.x;  // to turn shape white
			closest = abs(item.y - localGround);
			elevation = item.y;
		}
	}

	//cout << "FLOOR " << currShape << endl;
	if (elevation == 999)
		return 0;

	/* Do some manual ramp calulations */
	if (currShape == 94)
		localGround = interpolate(-31, -39, 0, 2.112, pos.z);
	else if (currShape == 41)
		localGround = interpolate(-39, -47.8, 2.112, 4.224, pos.z);
	else if (currShape == 612 || currShape == 364)
		localGround = interpolate(6.8, 3.0, -3.168, -4.224, pos.z);
	else if (currShape == 1150 || currShape == 978)
		localGround = interpolate(19.49, 6.99, 0, -3.168, pos.z);
	else if (currShape == 535 || currShape == 528 || currShape == 370)
		localGround = interpolate(-70.4, -76.5, -4.224, -2.112, pos.x);
	else if (currShape == 549 || currShape == 550 || currShape == 548)
		localGround = interpolate(-76.5, -82.9, -2.112, 0, pos.x);
	else if (currShape == 1387)
		localGround = interpolate(-57.8, -61.7, 3.168, 4.224, pos.x);
	else if (currShape == 1119)
		localGround = interpolate(-45, -57.8, 0, 3.168, pos.x);
	else if (currShape == 1415)
		localGround = 4.224;
	else if (currShape == 117)
		localGround = interpolate(-11, -15.5, 0, -1.056, pos.x);
	else if (currShape == 165)
		localGround = interpolate(-15.4, -28, -1.056, -4.224, pos.x);
	else if (currShape == 1358)
		localGround = interpolate(53.8, 49.4, 1.188, 0, pos.z);
	else if (currShape == 1481)
		localGround = interpolate(65.8, 53.8, 4.224, 1.188, pos.z);
	else if (currShape == 1283 || currShape == 1161)
		localGround = 1.056;
	else
		localGround = elevation;

	bool inWall = false;
	for (auto& it : walls)
	{
		vec3 min = it.second[0];
		vec3 max = it.second[1];

		if (nextPos.x > min.x && nextPos.x < max.x && nextPos.z > min.z && nextPos.z < max.z) {
			if (!std::count(excludeCollisionWall.begin(), excludeCollisionWall.end(), it.first)) {
				currShape = it.first;
				cout << "WALL " << it.first << endl;

				/* allow to jump over walls*/
				if (nextPos.y < max.y)
					inWall = true;
			}
		}
	}

	if (!inWall && !manualWall(nextPos)) 
		pos = nextPos;

	return currShape;
}

int Player::updatePos(vec3 lookAt, bool goCamera, float frametime, std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls)
{
	vec3 tempw;
	vec3 temps;
	vec3 tempa;
	vec3 tempd;
	vec3 tempv;

	float speednerf = 0.0;
	if (mvm_type == 0 && (abilityOneInUse || abilityTwoInUse))
		speednerf = 0.16;

	if (!goCamera) {
		if (w) 
			tempw = (speeds[mvm_type] - speednerf) * lookAt;
		if (!w)
			tempw = vec3(0, 0, 0);
		if (s)
			temps = -((speeds[mvm_type] - speednerf) * lookAt);
		if (!s)
			temps = vec3(0, 0, 0);
		if (a)
			tempa = -((speeds[mvm_type] - speednerf) * normalize(glm::cross(lookAt, up)));
		if (!a)
			tempa = vec3(0, 0, 0);
		if (d)
			tempd = (speeds[mvm_type] - speednerf) * normalize(glm::cross(lookAt, up));
		if (!d)
			tempd = vec3(0, 0, 0);

		// velocity = sum of WASD movements, but do not effect y direction
		tempv = tempw + temps + tempa + tempd;
		tempv.y = 0;


		// cap speed to movement speed type
		vel.x = std::min(tempv.x, speeds[mvm_type]);
		vel.z = std::min(tempv.z, speeds[mvm_type]);



		// Only involve gravity when player is jumping
		if (jumping) {
			vel = vel + acc * lastTime;

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
		}

		nextPos = pos + vel;
		pos = nextPos;
		//float res = collision(floors, walls);

		// Cap position (otherwise player sometimes goes into ground for a sec at the end of a jump)
		if (pos.y < localGround) {pos.y = localGround;}
		if (!jumping && pos.y > localGround) { pos.y = localGround; }

		return 0.0;

 	}
}


