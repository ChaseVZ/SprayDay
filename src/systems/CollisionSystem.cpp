#include "CollisionSystem.h"

using namespace std;
extern Coordinator gCoordinator;

// covert world coordinate to collision map index
int worldToMap(float val)
{
	/* val / TileSize; then truncate to an int */
	int s = MAP_SIZE;
	int res = static_cast<int> (val) + (s / 2) - 1;

	// Error checks
	// UPDATE: dont check here because world coords are used for math
	// checks are done in verifyCollisionAddition() and isCollision() 
	if (s % 2 == 1) { cout << "ERROR: Map size is not even!!" << endl; }

	return res;
}

int MaptoWorld(float val)
{
	return val - (MAP_SIZE / 2) + 1;
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

	localGround = interp;
	return false;
}

bool CollisionSys::checkHeight(int i, int j, vec3 pos, float* tempLocalGround)
{
	if (pos.y >= colMap[i][j].height - epsilon) // allow player to walk on top of objects
	{ 
		*tempLocalGround = colMap[i][j].height; 
		return false; 
	} 

	else 
	{ 
		// if player is in a ramp, ignore the crate in front of it
		if (ignoreDir.y == 1 && j > rampLoc.y || ignoreDir.x == 1 && i > rampLoc.x
			|| ignoreDir.y == -1 && j < rampLoc.y || ignoreDir.x == -1 && i < rampLoc.x) 
		{ return false; }

		printCol(vec2(i, j)); 
		return true; 
	}
}

void CollisionSys::setColDir(int i, int j) {
	bool outsideMap = false;
	if (i < 0 || i >= MAP_SIZE) { colDir.x = 0; outsideMap = true; }
	if (j < 0 || j >= MAP_SIZE) { colDir.z = 0; outsideMap = true; }
	if (outsideMap) { return; }

	vec3 colPos = colMap[i][j].center;
	float delta_i = colPos.x - entityPos.x;
	float delta_j = colPos.z - entityPos.z;

	//cout << "Delta i " << delta_i << " Delta j " << delta_j << endl;

	if (abs(delta_i) < abs(delta_j)) { // we need to remove vel in i dir
		//cout << "col in Z" << endl;
		colDir.z = 0;
	}
	else { // remove vel in j dir
		colDir.x = 0;
		//cout << "col in X" << endl;
	}

	//cout << "col dir is now " << colDir.x << " " << colDir.z << endl;

}

bool CollisionSys::isCollision(int i, int j, vec3 pos, bool* tempInRamp, float* tempLocalGround)
{
	if (i < 0 || j < 0) { setColDir(i, j); return true; } // don't let anything move outside of mapped world
	if (i >= MAP_SIZE || j >= MAP_SIZE) { setColDir(i, j); return true; } // don't let anything move outside of mapped world

	if ((colMap[i][j].c == 1 || colMap[i][j].c == 3))  // check map for crates & cubes
	{ 
		if (checkHeight(i, j, pos, tempLocalGround)) {
			setColDir(i, j);
			localGround = *tempLocalGround;
			return true;
		}
		localGround = *tempLocalGround;
		return false;
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

bool CollisionSys::checkCollide(vec3 nextPos, float radius)
{
	bool tempInRamp = false;
	float tempLocalGround = 0;
	playerInRamp = false;
	int k = 0;
	bool res = false;
	//entityPos = nextPos; // used to determine direction [potential] collision is occuring

	// top right corner	
	int i1 = worldToMap(nextPos.x + radius) - 1;
	int j1 = worldToMap(nextPos.z + radius) - 1;

	// bot left corner
	int i3 = worldToMap(nextPos.x - radius) + 1;
	int j3 = worldToMap(nextPos.z - radius) + 1;

	int i_extent = abs(i1 - i3) + 0;
	int j_extent = abs(j1 - j3) + 0;

	for (int _i = 0; _i <= i_extent; _i++) {
		for (int _j = 0; _j <= j_extent; _j++) {
			k++;
			if (isCollision(i3 + _i, j3 + _j, nextPos, &tempInRamp, &tempLocalGround)) 
			{ 
				//cout << "collisions checked: " << k << endl;
				res = true;
			}
			if (tempInRamp) { return false; }
		}
	}

	if (tempInRamp == false) { 
		ignoreDir = vec2(0, 0);
		rampLoc = vec2(0, 0);
		playerInRamp = false; 

		for (int _i = 0; _i <= i_extent; _i++) {
			for (int _j = 0; _j <= j_extent; _j++) {
				if (isCollision(i3 + _i, j3 + _j, nextPos, &tempInRamp, &tempLocalGround)) { return true; }
			}
		}
	
		localGround = tempLocalGround;
	}

	//cout << "collision dir is none" << endl;
	colDir = vec3(1); // vel is multiplied by colDir so 1,1,1 means no collision block
	return res;
}

// current;y just checks player collisions to static objects
CollisionOutput CollisionSys::checkCollisions(vec3 nextPos, bool isPlayer, vec3 curPos)
{
	isP = isPlayer;
	entityPos = curPos;
	bool res = checkCollide(nextPos, 3); // player radius hardcoded for now
	return CollisionOutput{ localGround, colDir, res, vec2(entityPos.x, entityPos.y) };

	//return res;
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
				collisions.push_back(vec2(MaptoWorld(i), MaptoWorld(j)));
			}
		}
		cout << "\n";
	}
	cout << endl << endl << endl;
	return collisions;
}
