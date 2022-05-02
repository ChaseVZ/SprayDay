
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


class Player {
public:
	Player();  // constructor
	vec3 updatePos(vec3 lookAt, bool goCamera, float frametime, bool *isMovingForward);
	void playerResetPos() { pos = pos_default; }
	vec3 pos;
	bool w, a, s, d, jumping;
	int mvm_type;
	bool debugMode = false;
	float localGround;
	float boRad;
	vec3 pos_default = vec3(0, 4.224, 0);
	vec3 win_loc = vec3(-41, -20, 70);


private:
	vec3 vel = vec3(0, 0, 0);  // velocity
	vec3 acc = vec3(0, -019.8, 0);	// acceleration
	vec3 jumpSpeed = vec3(0, 019.8 * 2, 0);  // jumpspeed (time = 2s)
	vec3 nextPos = vec3(0, 0, 0);
	float lastTime = 0;
	void checkCollision();
	vec3 oldMoveDir;
};


#endif