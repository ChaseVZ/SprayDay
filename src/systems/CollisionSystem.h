#include "../EcsCore/System.h"
#include "../Components/Collision.h"
#include "../GameManager.h"

// MAP_SIZE = 160;
// TILE_SIZE = 2;

class CollisionSys : public System
{
public:
	void init();
	bool checkCollisions(vec3 playerPos);

	float localGround; // output param of checkCollide()
	bool playerInRamp = false;
	vec2 ignoreDir = vec2(0, 0);
	vec2 rampLoc = vec2(0, 0);

private:
	CollisionComponent colMap[MAP_SIZE + 1][MAP_SIZE + 1] = {};
	void addStaticCollisions();
	void verifyCollisionAddition(int i, int j, CollisionComponent c);

	bool checkHeight(int i, int j, vec3 pos, float* tempLocalGround);
	bool isCollision(int i, int j, vec3 pos, bool* tempInRamp, float* tempLocalGround);
	bool checkCollide(vec3 pos, float radius);

	vec2 latestCol = vec2(-99, -99); // debug
	void printCol(vec2 newCol);

	bool interpRamp(vec3 pos, CollisionComponent cc);

};
