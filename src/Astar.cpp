#include <iostream>
#include "Astar.h"
#include "GameManager.h"
#include <map>
#include <vector>
#include <array>
#include <list>
#include <stack>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace glm;

const int EMPTY_BLOCK = 0;
const int CRATE_BLOCK = 1;
const int RAMP_BLOCK = 2;

//Developed with lots of help from https://dev.to/jansonsa/a-star-a-path-finding-c-4a4h and
// https://medium.com/@nicholas.w.swift/easy-a-star-pathfinding-7e6689c7f7b2


struct Node {
	vec3 pos;
	vec3 parentPos;
	float gCost;
	float hCost;
	float fCost;
	//bool isOpen = true;
};

inline bool operator < (const Node& lhs, const Node& rhs) {
	return lhs.fCost < rhs.fCost;
}

std::shared_ptr<CollisionSys> collisionSysAstar;

static int getBlockType(vec3 blockPos, shared_ptr<CollisionSys> collSys) {
	// 0 = empty space
	// 1 = crate
	// 2 = ramp
	blockPos = vec3(blockPos.x, 0, blockPos.z); // collison map is not vertical
	CollisionOutput colOut;
	colOut = collisionSysAstar->checkCollisions(collSys->mapToWorldVec(blockPos), false, vec3(-1));
	if (colOut.isCollide) {
		return CRATE_BLOCK;
	}
	else if (colOut.inRamp) {
		return RAMP_BLOCK;
	}
	else 
		return EMPTY_BLOCK;
}

static bool isDestination(vec3 newPos, vec3 destPos) {
	if (newPos== destPos) {
		return true;
	}
	return false;
}

static float euclideanDist(vec3 a, vec3 b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}

static float euclidApprox(vec3 a, vec3 b) {
	return (a.x - b.x) * (a.x - b.x) + ((a.y - b.y) * (a.y - b.y))*16.0 + (a.z - b.z) * (a.z - b.z);
}
static float calcH(vec3 newPos, Node dest) {
	return euclidApprox(newPos, dest.pos);
}

static vector<Node> makePath(array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2>* map, Node player) {
	int x = player.pos.x;
	int z = player.pos.z;
	int y = player.pos.y;
	stack<Node> path;
	vector<Node> usablePath; //reversed path from player->obj to obj->player
	while ((x != -1) && (z != -1)) {
		//cout << "  pushing: " << (*map)[y][x][z].pos.x << " " << (*map)[y][x][z].pos.y << " " << (*map)[y][x][z].pos.z << " " << endl;
		assert((*map)[y][x][z].parentPos != player.pos);
		path.push((*map)[y][x][z]);
		int tempX = x;
		int tempZ = z;
		x = (*map)[y][x][z].parentPos.x;
		z = (*map)[y][tempX][z].parentPos.z;
		y = (*map)[y][tempX][tempZ].parentPos.y;
	}
	while (!path.empty()) {
		Node top = path.top();
		path.pop();
		usablePath.emplace_back(top);
	}
	return usablePath;
}
bool isLessThan(Node a, Node b) {
	return a.fCost > b.fCost;
}

bool isDiagonal(int newX, int newZ) {
	return abs(newX) == 1 && abs(newZ) == 1;
}
void addTile(int x, int z, int y, int newX, int newZ, int newY, Node* node, Node* player,
	array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2>* map, 
	vector<Node>* openList) {
	double gNew, hNew, fNew;
	double gPrice = 1.0;
	if (isDiagonal(newX, newZ)) {
		gPrice = 3.0;
	}
	gNew = node->gCost + gPrice;
	hNew = calcH(vec3(x + newX, newY, z + newZ), *player);
	fNew = gNew + hNew;
	if ((*map)[newY][x + newX][z + newZ].fCost == FLT_MAX || (*map)[newY][x + newX][z + newZ].fCost > fNew) { //either unseen or improved
		(*map)[newY][x + newX][z + newZ].fCost = fNew;
		(*map)[newY][x + newX][z + newZ].gCost = gNew;
		(*map)[newY][x + newX][z + newZ].hCost = hNew;
		(*map)[newY][x + newX][z + newZ].parentPos.x = x;
		(*map)[newY][x + newX][z + newZ].parentPos.z = z;
		(*map)[newY][x + newX][z + newZ].parentPos.y = y;
		openList->push_back((*map)[newY][x + newX][z + newZ]);
		std::push_heap(openList->begin(), openList->end(), isLessThan);
	}
}
void addNeighbors(int x, int z, int y, array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2>* map, 
	std::shared_ptr<CollisionSys> collSys, bool visitedList[2][IDX_SIZE][IDX_SIZE], vector<Node>* openList,
	Node* node, Node* player) {
	for (int newX = -1; newX <= 1; newX++) {
		for (int newZ = -1; newZ <= 1; newZ++) {
			if (x + newX < 0 || z + newZ < 0 || x + newX >= IDX_SIZE || z + newZ >= IDX_SIZE) {
				break; // don't add tiles out of range
			}
			
			int blockType = getBlockType(vec3(x + newX, 0, z + newZ), collSys);
			//out << "  checking neighbor: " << x + newX << " " << y << " " << z + newZ << endl;
			if (y == 0) {
				if (visitedList[0][x + newX][z + newZ] == false && (blockType == EMPTY_BLOCK || blockType == RAMP_BLOCK)) {
					if (blockType == RAMP_BLOCK) {
						vec3 newPos = (*map)[1][x + newX][z + newZ].pos;
					}
					addTile(x, z, y, newX, newZ, 0, node, player, map, openList);
				}
				if (visitedList[1][x + newX][z + newZ] == false && (blockType == RAMP_BLOCK) /*&& !isDiagonal(newX, newZ)*/) {
					vec3 newPos = (*map)[1][x + newX][z + newZ].pos;
					addTile(x, z, y, newX, newZ, 1, node, player, map, openList);
				}
			}
			if (y == 1) {
				if (visitedList[1][x + newX][z + newZ] == false && (blockType == CRATE_BLOCK|| blockType == RAMP_BLOCK)) {
					addTile(x, z, y, newX, newZ, 1, node, player, map, openList);
				}
				if (visitedList[0][x + newX][z + newZ] == false && (blockType == RAMP_BLOCK) || blockType == EMPTY_BLOCK /*&& !isDiagonal(newX, newZ)*/) {
					addTile(x, z, y, newX, newZ, 0, node, player, map, openList);
				}
			}
		}
	}
}

static vector<Node> checkNodes(vec3 startPos, Node player, shared_ptr<CollisionSys> collSys) {
	vector<Node> empty;
	if ((getBlockType(player.pos, collSys) == CRATE_BLOCK) && player.pos.y == 0) { //player is unreachable and is in an obstacle
		return empty;
	}

	if (isDestination(startPos, player.pos)) { 
		return empty;
	}

	bool visitedList[2][IDX_SIZE][IDX_SIZE];

	//initialize map array to be filled in later
	array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2> * map = new array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2>;
	//cout << "initializing map" << endl;
	for (int x=0; x<IDX_SIZE; x++) {
		for (int z=0; z<IDX_SIZE; z++) {
			for (int y = 0; y < 2; y++) {
				(*map)[y][x][z].fCost = FLT_MAX;
				(*map)[y][x][z].gCost = FLT_MAX;
				(*map)[y][x][z].hCost = FLT_MAX;
				(*map)[y][x][z].parentPos.x = -1;
				(*map)[y][x][z].parentPos.z = -1;
				(*map)[y][x][z].parentPos.y = -1;
				(*map)[y][x][z].pos.x = x;
				(*map)[y][x][z].pos.z = z;
				(*map)[y][x][z].pos.y = y;
				visitedList[y][x][z] = false;
			}
		}
	}
	int x = startPos.x;
	int z = startPos.z;
	int y = startPos.y;
	(*map)[y][x][z].fCost = 0.0;
	(*map)[y][x][z].gCost = 0.0;
	(*map)[y][x][z].hCost = 0.0;

	vector<Node> openList;
	openList.push_back((*map)[y][x][z]);
	std::make_heap(openList.begin(), openList.end(), isLessThan);
	bool destinationFound = false;
	while (!openList.empty() && openList.size() < IDX_SIZE * IDX_SIZE) {
		Node selected = openList.front();
		std::pop_heap(openList.begin(), openList.end(), isLessThan);
		openList.pop_back();
		assert(selected.pos.x < IDX_SIZE && selected.pos.z < IDX_SIZE);
		assert(selected.pos.x >= 0 && selected.pos.z >= 0);
		assert(selected.fCost < 100000);
		x = selected.pos.x;
		z = selected.pos.z;
		y = selected.pos.y;
		visitedList[y][x][z] = true;
		if (isDestination(selected.pos, player.pos)) {
			destinationFound = true;
			vector<Node> returnVal = makePath(map, player);
			delete map;
			return returnVal;
		}
		//cout << " selected: " << x << " " << y << " " << z << endl;
		addNeighbors(x, z, y, map, collSys, visitedList, &openList, &selected, &player);
	}
	delete map;
	return empty;
}

bool vecIsLessThanOrEqual(vec3 a, vec3 b) {
	return euclideanDist(a, vec3(0, 0, 0)) <= euclideanDist(b, vec3(0, 0, 0));
}

bool vecIsGreaterThanOrEqual(vec3 a, vec3 b) {
	return euclideanDist(a, vec3(0, 0, 0)) > euclideanDist(b, vec3(0, 0, 0));
}

bool vecEpsilonEqual2(vec3 a, vec3 b, float epsilon) {
	if (abs(a.x - b.x) <= epsilon && abs(a.y - b.y) <= epsilon && abs(a.z - b.z) <= epsilon) {
		return true;
	}
	return false;
}
vec3 floorVec(vec3 inputVec) {
	return vec3(floor(inputVec.x), floor(inputVec.y), floor(inputVec.z));
}

vec3 initPlayerNodePos(vec3 truePlayerPos, shared_ptr<CollisionSys> collSys) {
	vec3 newPlayerPos = floorVec(collSys->worldToMapVec(truePlayerPos));
	if (newPlayerPos.y > 0) {
		int playerBlockType = getBlockType(newPlayerPos, collSys);
		if (playerBlockType == EMPTY_BLOCK) {
			newPlayerPos.y = 0; // enemies will chase the player's ground location while jumping
		}
		else {
			newPlayerPos.y = 1;
		}
	}
	return newPlayerPos;
}
vec3 Astar::findNextPos(Player p, Transform* tr, shared_ptr<CollisionSys> collSys) {
	//cerr << "inAstar\n";
	collisionSysAstar = collSys;
	Node playerNode;
	playerNode.pos = initPlayerNodePos(p.pos, collSys); 
	vec3 startPos; // startingPos
	startPos= floorVec(collSys->worldToMapVec(tr->pos));
	if (startPos.y > 1) {
		startPos.y = 1;
	}
	if (startPos.y < 0) {
		startPos.y = 0;
	}
	vector<Node> moves;
	assert(!(startPos.x >= IDX_SIZE || startPos.z >= IDX_SIZE));
	assert(!(startPos.z <= 0 || startPos.x <= 0));
	moves = checkNodes(startPos, playerNode, collSys);
	
	if (!moves.empty()){
		glm::vec3 retMove = collSys->mapToWorldVec(moves.front().pos);
		//cout << "   returned node: " << moves.front().pos.x << " " << moves.front().pos.z << endl;
		glm::vec3 trPos = floorVec(tr->pos);
		if (moves.size() > 1) { //retMove is same as pos, so return next pos in moveslist
			moves.erase(moves.begin());
			return collSys->mapToWorldVec(moves.front().pos * vec3(1.0, 4.0, 1.0));
			//return vec3(moves.front().pos.x-MAP_SIZE/2, 0, moves.front().pos.z-MAP_SIZE/2);
		}
	}
	//cout << "astar fail " << endl;
	return floorVec(tr->pos);
}
