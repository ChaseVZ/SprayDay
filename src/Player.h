
#ifndef CHASE_PLAYER
#define CHASE_PLAYER

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <vector>
#include "systems/CollisionSystem.h"

using namespace std;
using namespace glm;


class Player {
public:
	Player();  // constructor
	vec3 calcNextPos(vec3 lookAt, bool goCamera, float frametime, bool *isMovingForward);
	void updatePos(vec3 dirMask, bool isCollide, shared_ptr<CollisionSys> cs);
	void playerResetPos() { pos = pos_default; }
	vec3 pos;
	bool w, a, s, d, jumping;
	int mvm_type;
	bool debugMode = false;
	float localGround;
	float boRad;
	vec3 pos_default = vec3(20, 0, 0);
	vec3 win_loc = vec3(-41, -20, 70);
	vec3 nextPos = vec3(0, 0, 0);
	float health = 100.0;
	float maxHP = 100.0;
	vec3 vel = vec3(0, 0, 0);  // velocity


private:
	vec3 acc = vec3(0, -80.0, 0);	// acceleration
	vec3 jumpSpeed = vec3(0, 22.0, 0);  // jumpspeed (time = 2s)
	float lastTime = 0;
	void checkCollision();
	vec3 oldMoveDir;
	bool falling;
	vec3 colDir = vec3(1);
	float lastFrametime = 0;
	vec3 approxIntegrateAccel();
};


#endif