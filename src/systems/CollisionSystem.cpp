#include "CollisionSystem.h"
#include "../EcsCore/Coordinator.h"
#include "../Components/Transform.h"
#include <iostream>

using namespace std;
extern Coordinator gCoordinator;

// covert world coordinate to collision map index
int worldToMap(float val)
{
	/* val / TileSize; then truncate to an int */
	int s = MAP_SIZE;
	int res = static_cast<int> (val) + (s / 2);

	// Error checks
	// UPDATE: dont check here because world coords are used for math
	// checks are done in verifyCollisionAddition() and isCollision() 
	//if (res < 0) { cout << "ERROR: Negative collision index: " << val << endl; } // return -1;
	//if (res > s) { cout << "ERROR: Overload collision index: " << val << endl; } // return -1;
	if (s % 2 == 1) { cout << "ERROR: Map size is not even!!" << endl; }

	return res;
}

void CollisionSys::verifyCollisionAddition(int i, int j, CollisionT c)
{
	if (i >= 0 && j >= 0 && i <= MAP_SIZE && j <= MAP_SIZE) { colMap[i][j] = c; }
	//else{ cout << "not adding: >> i: " << i << " j: " << j << endl;}
}

void CollisionSys::addStaticCollisions()
{
	for (Entity const& entity : mEntities) {
		CollisionComponent& cc = gCoordinator.GetComponent<CollisionComponent>(entity);
		Transform& tc = gCoordinator.GetComponent<Transform>(entity);

		// top right
		int i1 = worldToMap(tc.pos.x + cc.length);
		int j1 = worldToMap(tc.pos.z + cc.width);

		// bot left
		int i3 = worldToMap(tc.pos.x - cc.length);
		int j3 = worldToMap(tc.pos.z - cc.width);

		// bot right
		int i2 = (i1 + i3 + j1 - j3) / 2;
		int j2 = (i3 - i1 + j1 + j3) / 2;

		// top left
		int i4 = (i1 + i3 + j3 - j1) / 2;
		int j4 = (i1 - i3 + j1 + j3) / 2;

		// verify corners
		verifyCollisionAddition(i1, j1, cc.c);
		verifyCollisionAddition(i2, j2, cc.c);
		verifyCollisionAddition(i3, j3, cc.c);
		verifyCollisionAddition(i4, j4, cc.c);

		// verify edges
		for (int k = 1; k < abs(j4 - j3) - 2 + 1; k++) {
			verifyCollisionAddition(i4, j4 - k, cc.c); // vertical R
			verifyCollisionAddition(i1, j1 - k, cc.c); // vetical L
			verifyCollisionAddition(i4 + k, j4, cc.c); // horizontal T
			verifyCollisionAddition(i3 + k, j3, cc.c); // horizontal B
		}
	}
}

void CollisionSys::init()
{
	addStaticCollisions();
}

void CollisionSys::printCol(vec2 newCol)
{
	if (newCol != latestCol)
	{
		latestCol = newCol;
		cout << "collision at >> i: " << latestCol.x << " j: " << latestCol.y << endl;
	}
}

bool CollisionSys::isCollision(int i, int j)
{
	if (i < 0 || j < 0) { return true; } // don't let anything move outside of mapped world
	if (i >= MAP_SIZE || j >= MAP_SIZE) { return true; } // don't let anything move outside of mapped world
	if (colMap[i][j] == 1) { printCol(vec2(i,j)); return true; } // check map
	return false;
}

bool CollisionSys::checkCollide(vec3 pos, float radius)
{
	int size = 0;

	// top right corner	
	int i1 = worldToMap(pos.x + radius);
	int j1 = worldToMap(pos.z + radius);
	if (isCollision(i1, j1)) { return true; }

	// bot left corner
	int i3 = worldToMap(pos.x - radius);
	int j3 = worldToMap(pos.z - radius);
	if (isCollision(i3, j3)) { return true; }

	// bot right corner
	int i2 = (i1 + i3 + j1 - j3) / 2;
	int j2 = (i3 - i1 + j1 + j3) / 2;
	if (isCollision(i2, j2)) { return true; }

	// top left corner
	int i4 = (i1 + i3 + j3 - j1) / 2;
	int j4 = (i1 - i3 + j1 + j3) / 2;
	if (isCollision(i4, j4)) { return true; }

	// tiles along vertical edges 
	for (int k = 1; k < abs(j4 - j3) - 2 + 1; k++) {
		if (isCollision(i4, j4 - k)) { return true; } // vertical R
		if (isCollision(i1, j1 - k)) { return true; } // vetical L
		if (isCollision(i4 + k, j4)) { return true; } // horizontal T
		if (isCollision(i3 + k, j3)) { return true; } // horizontal B
	}
	return false;
}

// current;y just checks player collisions to static objects
bool CollisionSys::checkCollisions(vec3 playerPos)
{
	//cout << playerPos.x << " " << playerPos.y << " " << playerPos.z << endl;
	bool res = checkCollide(playerPos, 2); // player radius hardcoded for now
	//cout << res << endl;
	return res;
}
