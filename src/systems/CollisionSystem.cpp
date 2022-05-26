#include "CollisionSystem.h"

using namespace std;
extern Coordinator gCoordinator;


// covert world coordinate to collision map index
int worldToMap(float val)
{
	/* val / TileSize; then truncate to an int */
	return floor(val + (MAP_SIZE / 2) - 1);
}

int mapToWorld(float val)
{
	return floor(val - ((MAP_SIZE / 2) -1));
}

vec3 CollisionSys::worldToMapVec(vec3 val) {
	return vec3(worldToMap(val.x), val.y, worldToMap(val.z));
}

vec3 CollisionSys::mapToWorldVec(vec3 val) {
	return vec3(mapToWorld(val.x), 0, mapToWorld(val.z));
}

void CollisionSys::verifyCollisionAddition(int i, int j, CollisionComponent cc)
{
	if (i >= 0 && j >= 0 && i <= MAP_SIZE && j <= MAP_SIZE) { colMap[i][j] = cc; }
}

void CollisionSys::addStaticCollisions()
{
	for (Entity const& entity : mEntities) {
		CollisionComponent& cc = gCoordinator.GetComponent<CollisionComponent>(entity);
		Transform& tc = gCoordinator.GetComponent<Transform>(entity);

		// top right
		int i1 = worldToMap(tc.pos.x + cc.length) - 1;
		int j1 = worldToMap(tc.pos.z + cc.width) - 1;

		// bot left
		int i3 = worldToMap(tc.pos.x - cc.length) + 1;
		int j3 = worldToMap(tc.pos.z - cc.width) + 1;

		int i_extent = abs(i1 - i3) + 0;
		int j_extent = abs(j1 - j3) + 0;

		for (int _i = 0; _i <= i_extent; _i++) {
			for (int _j = 0; _j <= j_extent; _j++) {
				verifyCollisionAddition(i3 + _i, j3 + _j, cc);
			}
		}
	}
}

void CollisionSys::fillEmpty() {
	for (int i = 0; i <= MAP_SIZE; i++) {
		for (int j = 0; j <= MAP_SIZE; j++) {
			if (colMap[i][j].c == 0) {
				CollisionComponent cc = CollisionComponent{ vec3(0), -1, -1, GROUND, 0 };
				colMap[i][j] = cc;
			}
		}
	}
}

void CollisionSys::init()
{
	if (MAP_SIZE % 2 == 1) { cout << "ERROR: Map size is not even!!" << endl; }
	addStaticCollisions();
}

void CollisionSys::printCol(vec2 newCol)
{
	if (newCol != latestCol)
	{
		latestCol = newCol; 
	}
}

void checkRampOrientation(float* y1, float* y2, CollisionComponent cc)
{
	if (cc.height <= 4.0f && (cc.dir.x == 1 || cc.dir.z == 1)) { *y2 = 0.0f; *y1 = cc.height; } // going from 0 > 4
	else if (cc.height <= 4.0f && (cc.dir.x == -1 || cc.dir.z == -1)) { *y2 = cc.height; *y1 = 0.0f; } // going from 4 > 0
	else if (cc.height <= 8.0f && (cc.dir.x == 1 || cc.dir.z == 1)) { *y2 = 4.0f; *y1 = cc.height; } // going from 4 > 8
	else if (cc.height <= 8.0f && (cc.dir.x == -1 || cc.dir.z == -1)) { *y2 = cc.height; *y1 = 4.0f; } // going from 8 > 4
}

void findInterpValue(float* x, CollisionComponent cc, vec3 pos)
{
	if (cc.dir == vec3(1, 0, 0) || cc.dir == vec3(-1,0,0)) { *x = pos.x; } // if ramp is oriented in x-axis, we interpolate on pos.x
	else if (cc.dir == vec3(0, 0, 1) || cc.dir == vec3(0,0,-1)) { *x = pos.z; } // if ramp is oriented in z-axis, we interpolate on pos.z
	else { cout << "invalid ramp dir: " << cc.dir.x << " " << cc.dir.y << " " << cc.dir.z << endl; }
}

float interpFunc(float x, float y1, float y2, float x1, float x2) { return y1 + (x - x1) * ((y2 - y1) / (x2 - x1)); }


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
		if (interp < y2) { interp = y2; }
		if (interp > y1) { interp = y1; }
	}

	// caps for negative facing ramps
	if (cc.dir.x == -1 || cc.dir.z == -1) {
		if (interp < y1) { interp = y1; }
		if (interp > y2) { interp = y2; }
	}

	if (pos.y < 0.3f && interp > pos.y && interp > pos.y + 1.2f) { setColDir(worldToMap(cc.center.x), worldToMap(cc.center.z)); return true; } // disallow teleporting up ramps
	localGround = interp;

	currSlope = cc.slope;

	//if (pos.y > 2.5f) { 
	//	cout << "yup\n";
	//	currSlope = interpFunc(x, cc.slope, 0, x1, x2); 
	//	if (currSlope < 0) { currSlope = 0; }
	//	if (currSlope > cc.slope) { currSlope = cc.slope; }
	//}
	//else { currSlope == cc.slope; }

	currSlopeDir = cc.dir;

	return false;
}

bool CollisionSys::collideOrIgnore(int i, int j, vec3 pos)
{
	//cout << playerInRamp << endl;
	//if (!playerInRamp && localGround < 4.0f && localGround > 0.0f) { localGround = 4.0f; cout << "here\n"; }

	if (pos.y >= colMap[i][j].height - epsilon) // allow player to walk on top of objects
	{ 
		// if player is in a ramp, we want to ignore setting localGround
		if (!InRamp)
			localGround = colMap[i][j].height;
		return false; 
	} 
	else { 
		/* if player is not above the crate, check if we can ignore it */

		// if player is not in a ramp, then we have a collision
		//if (!playerInRamp) { return true; }

		//cout << "rampBase: " << rampBase << " bounds: " << rampOrthoBounds.x << " " << rampOrthoBounds.y << endl;
		//cout << "ignoreDir: " << ignoreDir.x << " " << ignoreDir.y << endl;
		//cout << "ignoring: " << i << " " << j << endl << endl;

		// checks
		if (rampOrthoBounds.x < 0) { rampOrthoBounds.x++; }
		if (rampOrthoBounds.y >= MAP_SIZE) { rampOrthoBounds.y--; }

		if (ignoreDir.x == 1 && i > rampBase && j >= rampOrthoBounds.x && j <= rampOrthoBounds.y) {  return false; }
		if (ignoreDir.x == -1 && i < rampBase && j >= rampOrthoBounds.x && j <= rampOrthoBounds.y) { return false; }
		if (ignoreDir.y == 1 && j > rampBase && i >= rampOrthoBounds.x && i <= rampOrthoBounds.y) { return false; }
		if (ignoreDir.y == -1 && j < rampBase && i >= rampOrthoBounds.x && i <= rampOrthoBounds.y) { return false; }

		// if we can't ignore it either, we have a collision
		///cout << "cant ignore " << i << " " << j << endl << endl;
		//printCol(vec2(i, j)); 
		return true; 
	}
}

void CollisionSys::setColDir(int i, int j) {
	bool outsideMap = false;
	if (i < 0 || i >= MAP_SIZE) { colDir.x = 0; outsideMap = true; }
	if (j < 0 || j >= MAP_SIZE) { colDir.z = 0; outsideMap = true; }
	if (outsideMap) { return; }

	vec3 colPos = colMap[i][j].center;
	float delta_i = abs(colPos.x - entityPos.x);
	float delta_j = abs(colPos.z - entityPos.z);

	//cout << "Delta i " << delta_i << " Delta j " << delta_j << endl;

	/* Ignore slide collision when it exists more than a tile (4 units) away or if the epsilon is less than 1*/
	// might need to change last check to see if both values are greater than 3.0f? (instead of difference)
	//if (delta_i > 4.0f || delta_j > 4.0f || abs(delta_i - delta_j) < 1.0f) { return; }
	if ((delta_i > 2.9f && delta_j > 2.9f)) { return; } // delta_i > 4.0f || delta_j > 4.0f || 

	if (delta_i < delta_j) { // we need to remove vel in i dir
		//cout << "col in Z" << endl;
		colDir.z = 0;
	}
	else { // remove vel in j dir
		colDir.x = 0;
		//cout << "col in X" << endl;
	}
}

void CollisionSys::setRampInfo(CollisionComponent cc) {
	rampBase = worldToMap(cc.lowerBound);

	if (cc.dir.x == 1 || cc.dir.x == -1)
		rampOrthoBounds = vec2(worldToMap(cc.center.z - 1), worldToMap(cc.center.z + 1));
	else if (cc.dir.z == 1 || cc.dir.z == -1)
		rampOrthoBounds = vec2(worldToMap(cc.center.x - 1), worldToMap(cc.center.x + 1));
}

bool CollisionSys::isCollision(int i, int j, vec3 pos)
{
	if (i < 0 || j < 0) { setColDir(i, j); return true; } // don't let anything move outside of mapped world
	if (i >= MAP_SIZE || j >= MAP_SIZE) { setColDir(i, j); return true; } // don't let anything move outside of mapped world

	if ((colMap[i][j].c == 1 || colMap[i][j].c == 3))  // check map for crates & cubes
	{ 
		// either we collide
		if (collideOrIgnore(i, j, pos)) {
			setColDir(i, j);
			return true;
		}

		// or ignored the collision
		return false;
	}
	
	else if (colMap[i][j].c == 2) // check map for ramps (let player move on them)
	{ 
		return interpRamp(pos, colMap[i][j]); 
		//return false;
	} 

	return false;
}

bool CollisionSys::checkCollisionsAlg(vec3 nextPos, float length, float width)
{
	bool res = false;
	int ramps = 0;
	int cubes = 0;

	// top right corner	
	int i1 = worldToMap(nextPos.x + length) - 1;
	int j1 = worldToMap(nextPos.z + width) - 1;

	// bot left corner
	int i3 = worldToMap(nextPos.x - length) + 1;
	int j3 = worldToMap(nextPos.z - width) + 1;

	int i_extent = abs(i1 - i3) + 0;
	int j_extent = abs(j1 - j3) + 0;

	// Initial check to see if a ramp exists
	for (int _i = 0; _i <= i_extent; _i++) {
		for (int _j = 0; _j <= j_extent; _j++) {
			if (i3 + _i < 0 || j3 + _j < 0 || i3 + _i >= MAP_SIZE || j3 + _j >= MAP_SIZE) { continue; }
			if (colMap[i3 + _i][j3 + _j].c == 2) { 
				ramps++;
				InRamp = true;
				setRampInfo(colMap[i3 + _i][j3 + _j]); 
				ignoreDir = vec2(colMap[i3 + _i][j3 + _j].dir.x, colMap[i3 + _i][j3 + _j].dir.z);
			}
			else if (colMap[i3 + _i][j3 + _j].c == 3) {
				cubes++;
			}
		}
	}

	// if no cube is found, localGround remains 0
	// if ramp found, we ignore cube heights and just use interp ramp
	// if no ramp found, we use cube heights (to cause a collision or set height)

	//cout << k << endl;
	//if (k < 5 && playerInRamp) { playerInRamp = false; cout << "false\n\n"; }
	if (cubes > ramps && ramps != 0) { InRamp = false; localGround = 4.0; }

	for (int _i = 0; _i <= i_extent; _i++) {
		for (int _j = 0; _j <= j_extent; _j++) {
			if (isCollision(i3 + _i, j3 + _j, nextPos)) {
				res = true;
			}
		}
	}
	//if (isP)
	//	cout << playerInRamp << " " << cubes << " " << ramps << endl;
	if (cubes > ramps && ramps != 0) { localGround = 4.0; } // avoids falling into ramps when you are mostly off the ramp
	if ((InRamp && cubes > 3) || (!InRamp && ramps != 0) || (InRamp && ramps < 8 && cubes == 0)) {  // slight change when skunk reaches ends of a ramp
		//if (nextPos.y > 2.5f) {
		//	currSlopeDir.x = (interp(cubes, currSlopeDir.x, 0, 0, cubes + ramps));
		//	currSlopeDir.z = (interp(cubes, currSlopeDir.z, 0, 0, cubes + ramps));
		//}
		//else {
		//	currSlopeDir.x = (interp(cubes, 0, currSlopeDir.x, 0, cubes + ramps));
		//	currSlopeDir.z = (interp(cubes, 0, currSlopeDir.z, 0, cubes + ramps));
		//}
		//cout << "here\n";
		currSlope = radians(20.0f);
	}

	if (InRamp) { return false; }
	if (cubes < 6) { localGround = 0; return false; } //so you dont collide with corners & so you fall off cubes if you are partway off
	return res;
}

// current;y just checks player collisions to static objects
CollisionOutput CollisionSys::checkCollisions(vec3 nextPos, bool isPlayer, vec3 curPos)
{
	isP = isPlayer;
	entityPos = curPos;
	InRamp = false;
	colDir = vec3(1);
	ignoreDir = vec2(0);
	localGround = 0;
	currSlope = 0;
	currSlopeDir = vec3(0);

	bool res;

	if (isP) { res = checkCollisionsAlg(nextPos, 3, 3); } // player l x w hardcoded for now}
	else { res = checkCollisionsAlg(nextPos, 3, 3); } // enemy ; x w hardcoded for now}
	return CollisionOutput{ localGround, colDir, res, vec2(entityPos.x, entityPos.y), currSlope, currSlopeDir, InRamp };
}


bool CollisionSys::isCollisionPublic(vec3 pos) {
	//bool* unused = false;
	float* unused2 = 0;
	return checkCollisionsAlg(pos, 3, 3);
}

std::vector<vec2> CollisionSys::printMap(vec3 pos) {
	int i1 = worldToMap(pos.x);
	int j1 = worldToMap(pos.z);

	std::vector<vec2> collisions;

	for (int i = 0; i <= MAP_SIZE; i++) {
		for (int j = 0; j <= MAP_SIZE; j++) {
			if (i == i1 && j == j1 || i+1 == i1 && j+1 == j1 || i-1 == i1 && j-1 == j1 || i - 1 == i1 && j + 1 == j1 || i + 1 == i1 && j - 1 == j1)
				printf("X");
			else if (colMap[i][j].c == 0)
				printf(".");
			else {
				printf("%d", colMap[i][j].c);
				collisions.push_back(vec2(mapToWorld(i), mapToWorld(j)));
			}
		}
		cout << "\n";
	}
	cout << endl << endl << endl;
	return collisions;
}
