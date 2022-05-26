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

static int getBlockType(vec3 newPos, shared_ptr<CollisionSys> collSys) {
	// 0 = empty space
	// 1 = crate
	// 2 = ramp
	CollisionOutput colOut;
	colOut = collisionSysAstar->checkCollisions(collSys->mapToWorldVec(newPos), false, vec3(-1));
	if (colOut.isCollide) {
		return CRATE_BLOCK;
	}
	else if (colOut.inRamp) {
		return RAMP_BLOCK;
	}
	else 
		return EMPTY_BLOCK;
	//return !(collisionSysAstar->checkCollisions(newPos - vec3(MAP_SIZE/2 + 1, 0, MAP_SIZE/2 + 1), false, vec3(-1)).isCollide);
}

static bool isDestination(vec3 newPos, vec3 destPos) {
	if (newPos.x == destPos.x && newPos.z == destPos.z) {
		return true;
	}
	return false;
}

static float euclideanDist(vec3 a, vec3 b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
}

static float euclidApprox(vec3 a, vec3 b) {
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z);
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
	while ( ((*map)[y][x][z].parentPos.x != x || (*map)[y][x][z].parentPos.z != z) && (x != -1) && (z != -1)) {

		path.push((*map)[y][x][z]);
		int tempX = x;
		int tempZ = z;
		x = (*map)[y][x][z].parentPos.x;
		z = (*map)[y][tempX][z].parentPos.z;
		y = (*map)[y][tempX][tempZ].parentPos.y;
	}
	path.push((*map)[y][x][z]);
	while (!path.empty()) {
		Node top = path.top();
		path.pop();
		usablePath.emplace_back(top);
	}
	//cerr << "Made Path!\n";
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
	//cerr << "...it is valid\n";
				//calc new costs
	double gPrice = 1.0;
	if (isDiagonal(newX, newZ)) {
		gPrice = 3.0;
	}
	gNew = node->gCost + gPrice;
	hNew = calcH(vec3(x + newX, 0, z + newZ), *player);
	fNew = gNew + hNew;
	if ((*map)[newY][x + newX][z + newZ].fCost >= 10000) { //not on openList
		(*map)[newY][x + newX][z + newZ].fCost = fNew;
		(*map)[newY][x + newX][z + newZ].gCost = gNew;
		(*map)[newY][x + newX][z + newZ].hCost = hNew;
		(*map)[newY][x + newX][z + newZ].parentPos.x = x;
		(*map)[newY][x + newX][z + newZ].parentPos.z = z;
		(*map)[newY][x + newX][z + newZ].parentPos.y = y;
		//cout << " pushing " << endl;
		openList->push_back((*map)[newY][x + newX][z + newZ]);
		//cout << "successful push "<< endl;
		std::push_heap(openList->begin(), openList->end(), isLessThan);
		//cerr << "adding tile " << x+newX << " " << z + newZ << " to open List\n";
	}
	else if ((*map)[0][x][z].gCost > gNew) { //already on openList
		(*map)[newY][x + newX][z + newZ].parentPos.x = x;
		(*map)[newY][x + newX][z + newZ].parentPos.z = z;
		(*map)[newY][x + newX][z + newZ].parentPos.y = y;
		(*map)[newY][x + newX][z + newZ].gCost = node->gCost + 1.0;
		(*map)[newY][x + newX][z + newZ].hCost = calcH(vec3(x + newX, 0, z + newZ), *player);
		(*map)[newY][x + newX][z + newZ].fCost = gNew + hNew;
	}
}
void addNeighbors(int x, int z, int y, array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2>* map, 
	std::shared_ptr<CollisionSys> collSys, bool visitedList[2][IDX_SIZE][IDX_SIZE], vector<Node>* openList,
	Node* node, Node* player) {
	for (int newX = -1; newX <= 1; newX++) {
		for (int newZ = -1; newZ <= 1; newZ++) {

			// don't add tiles out of range
			if (x + newX < 0 || z + newZ < 0 || x + newX >= IDX_SIZE || z + newZ >= IDX_SIZE) {
				break;
			}
			int blockType = getBlockType(vec3(x + newX, 0, z + newZ), collSys);
			if (y == 0) {
				if (visitedList[0][x + newX][z + newZ] == false && (blockType == EMPTY_BLOCK || blockType == RAMP_BLOCK)) {
					if (blockType == RAMP_BLOCK) {
						vec3 newPos = (*map)[1][x + newX][z + newZ].pos;
						//cout << "adding rampi: " << newPos.y << " " << newPos.x << " " << newPos.z << endl;
					}
					addTile(x, z, y, newX, newZ, 0, node, player, map, openList);
				}
				/*
				if (blockType == RAMP_BLOCK) {
					cout << "vert visited?" << visitedList[1][x + newX][z + newZ] << endl;
				}*/
				if (visitedList[1][x + newX][z + newZ] == false && (blockType == RAMP_BLOCK) /*&& !isDiagonal(newX, newZ)*/) {
					vec3 newPos = (*map)[1][x + newX][z + newZ].pos;
					//cout << "adding vertical tile: " << newPos.y << " " << newPos.x <<" " << newPos.z << endl;
					addTile(x, z, y, newX, newZ, 1, node, player, map, openList);
				}
			}
			if (y == 1) {
				if (visitedList[1][x + newX][z + newZ] == false && (blockType == CRATE_BLOCK|| blockType == RAMP_BLOCK)) {
					addTile(x, z, y, newX, newZ, 1, node, player, map, openList);
				}
				if (visitedList[0][x + newX][z + newZ] == false && (blockType == RAMP_BLOCK) /*&& !isDiagonal(newX, newZ)*/) {
					addTile(x, z, y, newX, newZ, 1, node, player, map, openList);
				}
			}
		}
	}
}

static vector<Node> checkNodes(Node startNode, Node player, shared_ptr<CollisionSys> collSys) {
	vector<Node> empty;
	if (!(getBlockType(player.pos, collSys) == EMPTY_BLOCK) && player.pos.y == 0) { //player is unreachable and is in an obstacle
		//Player is in obstacle!
		return empty;
	}

	if (isDestination(startNode.pos, player.pos)) { 
		//cout << "Reached destination! You were already there :)\n";
		return empty;
	}

	bool visitedList[2][IDX_SIZE][IDX_SIZE];

	//initialize map array to be filled in later
	array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2> * map = new array<array<array<Node, IDX_SIZE>, IDX_SIZE>, 2>;

	for (int x=0; x<IDX_SIZE; x++) {
		for (int z=0; z<IDX_SIZE; z++) {
			for (int y = 0; y < 2; y++) {
				(*map)[y][x][z].fCost = FLT_MAX;
				(*map)[y][x][z].gCost = FLT_MAX;
				(*map)[y][x][z].hCost = FLT_MAX;
				(*map)[y][x][z].parentPos.x = -1;
				(*map)[y][x][z].parentPos.z = -1;
				(*map)[y][x][z].pos.x = x;
				(*map)[y][x][z].pos.z = z;
				(*map)[y][x][z].pos.y = y;
				visitedList[y][x][z] = false;
			}
		}
	}

	//init starting list
	int x = startNode.pos.x;
	int z = startNode.pos.z;
	int y = startNode.pos.y;
	(*map)[y][x][z].fCost = 0.0;
	(*map)[y][x][z].gCost = 0.0;
	(*map)[y][x][z].hCost = 0.0;
	(*map)[y][x][z].parentPos.x = x;
	(*map)[y][x][z].parentPos.z = z;

	vector<Node> openList;
	openList.push_back((*map)[y][x][z]);
	std::make_heap(openList.begin(), openList.end(), isLessThan);
	bool destinationFound = false;
	while (!openList.empty() && openList.size() < IDX_SIZE * IDX_SIZE) {
		float temp = FLT_MAX;
		Node node;
		/*
		cout << "OpenList\n";
		for (Node n : openList) {
			cout << "    Node: " << n.pos.y <<" " << n.pos.x << " " << n.pos.z << " " << n.fCost << "\n";
		}*/
		
		node = openList.front();
		//cout << "Picked Node: " << node.pos.y <<" " << node.pos.x << " " << node.pos.z << " "  << endl;
		std::pop_heap(openList.begin(), openList.end(), isLessThan);
		openList.pop_back();
		assert(node.pos.x < IDX_SIZE && node.pos.z < IDX_SIZE);
		assert(node.pos.x >= 0 && node.pos.z >= 0);
		assert(node.fCost < 100000);
		x = node.pos.x;
		z = node.pos.z;
		y = node.pos.y;
		visitedList[y][x][z] = true;
		if (isDestination(node.pos, player.pos)) {
			destinationFound = true;
			cout << "making path" << endl;
			vector<Node> returnVal = makePath(map, player);
			cout << "made path" << endl;
			delete map;
			return returnVal;
		}
		addNeighbors(x, z, y, map, collSys, visitedList, &openList, &node, &player);
	}
	//if (destinationFound == false) {
			//cout << "Did not find player: " << openList.size() << " nodes left to search\n";
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
vec3 truncateVec(vec3 inputVec) {
	return vec3(floor(inputVec.x), floor(inputVec.y), floor(inputVec.z));
}
vec3 Astar::findNextPos(Player p, Transform* tr, shared_ptr<CollisionSys> collSys) {
	//cerr << "inAstar\n";
	collisionSysAstar = collSys;
	Node player;
	player.pos = truncateVec(collSys->worldToMapVec(p.pos));
	if (player.pos.y > 1) {
		player.pos.y = 1;
	}
	cout << "player pos: " << player.pos.y << " " << player.pos.x << " " << player.pos.z << endl;
	//player.pos = vec3(round(p.pos)) + vec3(MAP_SIZE/2, 0, MAP_SIZE/2); //convert from world coors to map coords
	/*
	if (player.pos.y > 0) {
		return truncateVec(tr->pos);
	}
	*/
	Node startNode; // startingPos
	startNode.pos = collSys->worldToMapVec(tr->pos);
	vector<Node> moves;
	assert(!(startNode.pos.x >= IDX_SIZE || startNode.pos.z >= IDX_SIZE));
	assert(!(startNode.pos.z <= 0 || startNode.pos.x <= 0));
	moves = checkNodes(startNode, player, collSys);
	if (!moves.empty()){
		glm::vec3 retMove = collSys->mapToWorldVec(moves.front().pos);
		glm::vec3 trPos = truncateVec(tr->pos);
		if (moves.size() > 1) { //retMove is same as pos, so return next pos in moveslist
			moves.erase(moves.begin());
			return collSys->mapToWorldVec(moves.front().pos);
			//return vec3(moves.front().pos.x-MAP_SIZE/2, 0, moves.front().pos.z-MAP_SIZE/2);
		}
	}
	//cout << "astar fail" << endl;
	return truncateVec(tr->pos);
}
