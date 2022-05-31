#pragma once
#include "../EcsCore/System.h"
#include "../Components/Collision.h"
#include "../GameManager.h"
#include "../EcsCore/Coordinator.h"
#include "../Components/Transform.h"
#include <iostream>
#include <vector>



// MAP_SIZE = 160;
// TILE_SIZE = 2;

class CollisionSys : public System
{
public:
	void init();
	CollisionOutput checkCollisions(vec3 nextPos, bool isPlayer, vec3 curPos);
	bool isCollisionPublic(vec3 pos);
	std::vector<vec2> printMap(vec3 pos); // debug
	void       printFastColMap(vec3 pos); // debug

	vec3 worldToMapVec(vec3 val);
	vec3 mapToWorldVec(vec3 val);
	int CollisionSys::getBlockTypeForEnemy(int i, int j);

private:
	float localGround; // output param of checkCollide()
	vec3 colDir = vec3(1); // used with entityPos
	bool InRamp = false;
	vec2 ignoreDir = vec2(0);  // used to ignore cube in front of a ramp
	vec3 entityPos = vec3(0); // used to determine direction at which collision is occurring 
	float currSlope = 0;
	vec3 currSlopeDir = vec3(0);
	CollisionComponent colMap[MAP_SIZE + 1][MAP_SIZE + 1] = {};
	int fastColMap[MAP_SIZE + 1][MAP_SIZE + 1] = {};

	float rampBase;
	vec2 rampOrthoBounds;

	float epsilon = 0.60f;

	bool isP = false;

	/* Collision Init */
	void addStaticCollisions();
	void initFastColMap();
	void fillEmpty();
	void verifyCollisionAddition(int i, int j, CollisionComponent c);

	/* HELPERS */
	void setColDir(int i, int j);
	void setRampInfo(CollisionComponent cc);
	bool interpRamp(vec3 pos, CollisionComponent cc);
	bool collideOrIgnore(int i, int j, vec3 pos);
	//bool isCollision(int i, int j, vec3 pos, bool* tempInRamp, float* tempLocalGround);
	
	/* Main Functions */
	bool isCollision(int i, int j, vec3 pos);
	bool checkCollisionsAlg(vec3 pos, float length, float width);

	/* DEBUG */
	vec2 latestCol = vec2(-99, -99);
	void printCol(vec2 newCol);
};
