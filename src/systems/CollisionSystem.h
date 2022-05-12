#include "../EcsCore/System.h"
#include "../Components/Collision.h"
#include "../GameManager.h"

// MAP_SIZE = 160;
// TILE_SIZE = 2;

class CollisionSys : public System
{
public:
	void init();
	CollisionOutput checkCollisions(vec3 playerPos);

private:
	float localGround; // output param of checkCollide()
	vec3 colDir = vec3(1); // used with entityPos
	bool playerInRamp = false;
	vec2 ignoreDir = vec2(0); 
	vec2 rampLoc = vec2(0); // used to determine which crate collisions need to be ignored (for that ramp)
	vec3 entityPos = vec3(0); // used to determine direction at which collision is occurring 
	CollisionComponent colMap[MAP_SIZE + 1][MAP_SIZE + 1] = {};

	void addStaticCollisions();
	void verifyCollisionAddition(int i, int j, CollisionComponent c);

	void CollisionSys::setColDir(vec3 colPos);
	bool CollisionSys::checkHeight(int i, int j, vec3 pos, float* tempLocalGround);
	bool isCollision(int i, int j, vec3 pos, bool* tempInRamp, float* tempLocalGround);
	bool checkCollide(vec3 pos, float radius);

	vec2 latestCol = vec2(-99, -99); // debug
	void printCol(vec2 newCol);

	bool interpRamp(vec3 pos, CollisionComponent cc);

};
