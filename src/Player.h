#pragma once
#ifndef CHASE_PLAYER
#define CHASE_PLAYER

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <vector>

using namespace std;
using namespace glm;

class Arrow {
public:
	Arrow();
	void setVelocity(float holdTime, vec3 lookAt);
	void update(float frametime, vec3 playerPos, vec3 lookAt, std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls);
	vec3 pos;
	vec3 vel;
	int charges;
	bool instanced = false;
	bool debugMode = false;
	bool landed = false;
	float rotationX = 0.0;
	float rotationZ = 0.0;
	float rotationY = 0.0;
	quat myQuaternion;

private:
	bool shot = false;
	vec3 acc = vec3(0, -0.098, 0);
	float startRotationZ = 120.0;
	float endRotationZ = 20.0;
	float startRotationY = 45.0;
	float endRotationY = -45.0;
	float startRotationX = 120.0;
	float endRotationX = -20.0;
	float lastTime = 0;
	float totalTime = 0;	// calculate in setVelocity()
	float lookAtZ_Store = 0;
	float lookAtX_Store = 0;
	bool collision(std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls, vec3 nextPos);
	vector<int> excludeCollisionWall = { 75, 788, 42, 265, 1518, 44, 92, 98, 52, 189, 289, 171, 1001, 324, 614, 657, 599, 1514, 649,
										361, 593, 605, 325, 1070, 1068, 292, 344, 591, 1164, 1403, 1480, 211, 1459, 1333, 1278, 223,
										219, 243, 1314, 1316, 1311, 745, 1381, 1172, 691, 488 };
	vector<int> excludeCollisionFloor = { 916, 919, 148, 139, 155, 1520, 87, 194, 198, 90, 195, 211, 824, 835, 1524, 1012, 617,
											600, 1516, 644, 656, 767, 961, 327, 310, 1207, 1125, 1103, 1101, 1307, 1112, 1449, 1435, 1448,
											1494, 996, 1085, 1091, 1075, 1095, 1370, 1361, 1501, 1489, 1386, 1437, 1502, 1482, 212, 215, 1352,
											1158, 1340, 1348, 652, 722, 629, 583, 598, 1385, 1366, 1384, 299, 911, 825, 1464, 1409, 1115 };
};

class Bullet {
public:
	Bullet(vec3 p, vec3 r);
	bool update(vector<vec3>& enemyPositions);		// returns TRUE if bullet can be despawned
	vec3 pos;

private:
	vec3 ray;
	float speedFactor = 1.5;
};


class Gun {
public:
	Gun();
	vec3 pos;
	vector<shared_ptr<Bullet>> bullets;
	void update(float frametime, vec3 pos, vec3 lookAt, bool playerShooting, vector<vec3>& enemyPositions);

private:
	float timeSinceLastBullet = 0;
	float shootThreshold = 0.2; 
};


class Player {
public:
	Player();  // constructor
	int updatePos(vec3 lookAt, bool goCamera, float frametime, std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls);
	void playerResetPos() { pos = pos_default; }
	vec3 pos;
	bool w, a, s, d, jumping;
	int mvm_type;
	bool abilityOneInUse; 
	bool abilityTwoInUse;
	bool debugMode = false;
	float localGround;
	vec3 pos_default = vec3(-91.226, 4.224, 72.539);
	vec3 win_loc = vec3(-41, -20, 70);
	Arrow arrow = Arrow();
	Gun rifle = Gun();


private:
	vec3 vel = vec3(0, 0, 0);  // velocity
	vec3 acc = vec3(0, -0.098, 0);	// acceleration
	vec3 jumpSpeed = vec3(0, 0.098 * 2, 0);  // jumpspeed (time = 2s)
	vec3 nextPos = vec3(0, 0, 0);
	vector<int> excludeCollisionWall = { 75, 788, 42, 265, 1518, 44, 92, 98, 52, 189, 289, 171, 1001, 324, 614, 657, 599, 1514, 649,
											361, 593, 605, 325, 1070, 1068, 292, 344, 591, 1164, 1403, 1480, 211, 1459, 1333, 1278, 223,
											219, 243, 1314, 1316, 1311, 745, 1381, 1172, 691, 488};
	vector<int> excludeCollisionFloor = { 916, 919, 148, 139, 155, 1520, 87, 194, 198, 90, 195, 211, 824, 835, 1524, 1012, 617,
											600, 1516, 644, 656, 767, 961, 327, 310, 1207, 1125, 1103, 1101, 1307, 1112, 1449, 1435, 1448,
											1494, 996, 1085, 1091, 1075, 1095, 1370, 1361, 1501, 1489, 1386, 1437, 1502, 1482, 212, 215, 1352,
											1158, 1340, 1348, 652, 722, 629, 583, 598, 1385, 1366, 1384, 299, 911, 825};
	float lastTime = 0;
	int collision(std::map<int, vector<vec3>> floors, std::map<int, vector<vec3>> walls);
};


#endif