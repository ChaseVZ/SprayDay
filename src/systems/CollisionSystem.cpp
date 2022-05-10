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

void CollisionSys::verifyCollisionAddition(int i, int j, CollisionComponent cc)
{
	if (i >= 0 && j >= 0 && i <= MAP_SIZE && j <= MAP_SIZE) { colMap[i][j] = cc; }
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
		verifyCollisionAddition(i1, j1, cc);
		verifyCollisionAddition(i2, j2, cc);
		verifyCollisionAddition(i3, j3, cc);
		verifyCollisionAddition(i4, j4, cc);

		// verify edges
		for (int k = 1; k < abs(j4 - j3) - 2 + 1; k++) {
			verifyCollisionAddition(i4, j4 - k, cc); // vertical R
			verifyCollisionAddition(i1, j1 - k, cc); // vetical L
			verifyCollisionAddition(i4 + k, j4, cc); // horizontal T
			verifyCollisionAddition(i3 + k, j3, cc); // horizontal B
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

void checkRampOrientation(float* y1, float* y2, CollisionComponent cc)
{
	if (cc.height <= 4.0f && (cc.dir.x == 1 || cc.dir.z == 1)) { *y2 = 0.0f; *y1 = cc.height; } // going from 0 > 4
	else if (cc.height <= 4.0f && (cc.dir.x == -1 || cc.dir.z == -1)) { *y2 = cc.height; *y1 = 0.0f; cout << "going down" << endl; } // going from 4 > 0
	else if (cc.height <= 8.0f && (cc.dir.x == 1 || cc.dir.z == 1)) { *y2 = 4.0f; *y1 = cc.height; } // going from 4 > 8
	else if (cc.height <= 8.0f && (cc.dir.x == -1 || cc.dir.z == -1)) { *y2 = cc.height; *y1 = 4.0f; } // going from 8 > 4
}

void findInterpValue(float* x, CollisionComponent cc, vec3 pos)
{
	if (cc.dir == vec3(1, 0, 0) || cc.dir == vec3(-1,0,0)) { *x = pos.x; } // if ramp is oriented in x-axis, we interpolate on pos.x
	else if (cc.dir == vec3(0, 0, 1) || cc.dir == vec3(0,0,-1)) { *x = pos.z; } // if ramp is oriented in z-axis, we interpolate on pos.z
	else { cout << "invalid ramp dir: " << cc.dir.x << " " << cc.dir.y << " " << cc.dir.z << endl; }
}


// return T if attempting to enter ramp from base
bool CollisionSys::interpRamp(vec3 pos, CollisionComponent cc)
{
	float y1 = 0;
	float y2 = 0;
	float x = 0;
	float x1 = cc.upperBound;
	float x2 = cc.lowerBound;

	checkRampOrientation(&y1, &y2, cc); // find min and max y value

	findInterpValue(&x, cc, pos); // find value we interpolate on (either pos.x, or pos.z)
	
	float interp = y1 + (x - x1) * ((y2 - y1) / (x2 - x1)); // INTERPOLATION FUNCTION

	/* ## CAP BOUNDS OF INTERP FUNCTION ## */
	// caps for positive facing ramps
	if (cc.dir.x == 1 || cc.dir.z == 1) {
		if (interp < y2) { interp = y2; cout << "pos x y2" << endl;}
		if (interp > y1) { interp = y1; cout << "pos x y1" << endl;}
	}

	// caps for negative facing ramps
	if (cc.dir.x == -1 || cc.dir.z == -1) {
		if (interp < y1) { interp = y1; cout << "neg x y1" << endl; }
		if (interp > y2) { interp = y2; cout << "neg x y2" << endl; }
	}

	cout << "local ground (ramp) to: " << interp << endl;
	localGround = interp;

	cout << "val: " << x << " y1: " << y1 << " y2: " << y2 << " x1: " << x1 << " x2: " << x2 << " localGround: " << localGround << endl;
	return false;
}

bool CollisionSys::checkHeight(int i, int j, vec3 pos, float* tempLocalGround)
{
	//cout << "COLLISION - player height = " << pos.y << " and col height: " << colMap[i][j].height << " @:" << i << " " << j << endl;

	if (pos.y >= colMap[i][j].height) // allow player to walk on top of objects
	{ 
		*tempLocalGround = colMap[i][j].height; 
		cout << "local ground (cube) to: " << *tempLocalGround << endl;
		return false; 
	} 

	else 
	{ 
		// if player is in a ramp, ignore the crate in front of it
		if (ignoreDir.y == 1 && j > rampLoc.y) { cout << "ignoring crate in front of Z+ ramp" << endl; return false; }
		else if (ignoreDir.x == 1 && i > rampLoc.x) { cout << "ignoring crate in front of X+ ramp" << endl; return false; }
		else if (ignoreDir.y == -1 && j < rampLoc.y) { cout << "ignoring crate in front of Z- ramp" << endl; return false; }
		else if (ignoreDir.x == -1 && i < rampLoc.x) { cout << "ignoring crate in front of X- ramp" << endl; return false; }

		printCol(vec2(i, j)); 
		return true; 
	}
}

bool CollisionSys::isCollision(int i, int j, vec3 pos, bool* tempInRamp, float* tempLocalGround)
{
	if (i < 0 || j < 0) { return true; } // don't let anything move outside of mapped world
	if (i >= MAP_SIZE || j >= MAP_SIZE) { return true; } // don't let anything move outside of mapped world

	if ((colMap[i][j].c == 1 || colMap[i][j].c == 3))  // check map for crates & cubes
	{ 
		return checkHeight(i, j, pos, tempLocalGround);
	}
	
	else if (colMap[i][j].c == 2) // check map for ramps (let player move on them)
	{ 
		rampLoc = vec2(i, j);
		ignoreDir = vec2(colMap[i][j].dir.x, colMap[i][j].dir.z);
		*tempInRamp = true;
		playerInRamp = true;
		return interpRamp(pos, colMap[i][j]); 
	} 

	return false;
}

bool CollisionSys::checkCollide(vec3 pos, float radius)
{
	int size = 0;
	bool tempInRamp = false;
	float tempLocalGround = 0;

	// top right corner	
	int i1 = worldToMap(pos.x + radius);
	int j1 = worldToMap(pos.z + radius);
	if (isCollision(i1, j1, pos, &tempInRamp, &tempLocalGround)) { return true; }

	// bot left corner
	int i3 = worldToMap(pos.x - radius);
	int j3 = worldToMap(pos.z - radius);
	if (isCollision(i3, j3, pos, &tempInRamp, &tempLocalGround)) { return true; }

	// bot right corner
	int i2 = (i1 + i3 + j1 - j3) / 2;
	int j2 = (i3 - i1 + j1 + j3) / 2;
	if (isCollision(i2, j2, pos, &tempInRamp, &tempLocalGround)) { return true; }

	// top left corner
	int i4 = (i1 + i3 + j3 - j1) / 2;
	int j4 = (i1 - i3 + j1 + j3) / 2;
	if (isCollision(i4, j4, pos, &tempInRamp, &tempLocalGround)) { return true; }

	// tiles along vertical edges 
	for (int k = 1; k < abs(j4 - j3) - 2 + 1; k++) {
		if (isCollision(i4, j4 - k, pos, &tempInRamp, &tempLocalGround)) { return true; } // vertical R
		if (isCollision(i1, j1 - k, pos, &tempInRamp, &tempLocalGround)) { return true; } // vetical L
		if (isCollision(i4 + k, j4, pos, &tempInRamp, &tempLocalGround)) { return true; } // horizontal T
		if (isCollision(i3 + k, j3, pos, &tempInRamp, &tempLocalGround)) { return true; } // horizontal B
	}

	if (tempInRamp == false) { 
		ignoreDir = vec2(0, 0);
		rampLoc = vec2(0, 0);
		playerInRamp = false; 
		//localGround = 0;

		if (isCollision(i1, j1, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; }
		if (isCollision(i3, j3, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; }
		if (isCollision(i2, j2, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; }
		if (isCollision(i4, j4, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; }

		for (int k = 1; k < abs(j4 - j3) - 2 + 1; k++) {
			if (isCollision(i4, j4 - k, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; } // vertical R
			if (isCollision(i1, j1 - k, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; } // vetical L
			if (isCollision(i4 + k, j4, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; } // horizontal T
			if (isCollision(i3 + k, j3, pos, &tempInRamp, &tempLocalGround)) { localGround = tempLocalGround; return true; } // horizontal B
		}

		//cout << "local ground (final) to: " << tempLocalGround << endl;
		localGround = tempLocalGround;
	}

	return false;
}

// current;y just checks player collisions to static objects
bool CollisionSys::checkCollisions(vec3 playerPos)
{
	bool res = checkCollide(playerPos, 2); // player radius hardcoded for now

	return res;
}
