#include "../EcsCore/System.h"
#include "../Components/Collision.h"
#include "../GameManager.h"

//const int MAP_SIZE = 160; // world bounds = -MAP_SIZE/2 to +MAP_SIZE/2
//const int TILE_SIZE = 2; // crates take up a 1x1 * crate_size area in world space

class CollisionSys : public System
{
public:
	void init();
	bool checkCollisions(vec3 playerPos);

	//int getSize() { return MAP_SIZE; }
	//int getTileSize() { return TILE_SIZE; }

private:
	//int MAP_SIZE = GameManager::GetInstance()->getSize();
	int colMap[MAP_SIZE + 1][MAP_SIZE + 1] = {};
	void addStaticCollisions();
	void verifyCollisionAddition(int i, int j, CollisionT c);

	bool isCollision(int i, int j);
	bool checkCollide(vec3 pos, float radius);

	vec2 latestCol = vec2(-99, -99); // debug
	void printCol(vec2 newCol);
};
