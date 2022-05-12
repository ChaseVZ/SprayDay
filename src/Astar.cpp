#include <iostream>
#include "Astar.h"
#include "GameManager.h"
#include <map>
#include <vector>
#include <array>
#include <list>
#include <stack>
#include <iostream>

using namespace std;
using namespace glm;

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

static bool isValid(vec3 newPos) {
	//cout << "isValid\n";

	return !collisionSysAstar->checkCollisions(newPos-vec3(MAP_SIZE/2, 0, MAP_SIZE/2));
}

static bool isDestination(vec3 newPos, Node dest) {
	//cout << "isDestination\n";
	if (newPos == dest.pos) {
		return true;
	}
	return false;
}

static float euclideanDist(vec3 a, vec3 b) {
	return sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y) + (a.z-b.z)*(a.z-b.z));
}

static float calcH(vec3 newPos, Node dest) {
	return euclideanDist(newPos, dest.pos);
}

static vector<Node> makePath(array<array<Node, MAP_SIZE>, MAP_SIZE> map, Node player) {
	//cerr << "Making Path!\n";
	int x = player.pos.x;
	int z = player.pos.z;
	stack<Node> path;
	vector<Node> usablePath; //reversed path from player->obj to obj->player
    //cerr << "Making Path! line62\n";
	while ((map[x][z].parentPos.x != x || map[x][z].parentPos.z != z) && x != -1 && z != -1) {
		//cerr << "path: " << x << " " << z << "\n";
		path.push(map[x][z]);
		x = map[x][z].parentPos.x;
		z = map[x][z].parentPos.z;
		
	}
	//cerr << "Making Path! line70\n";
	path.push(map[x][z]);
	while (!path.empty()) {
		Node top = path.top();
		path.pop();
		usablePath.__emplace_back(top);
	}
	//cerr << "Made Path!\n";
	return usablePath;
}


static vector<Node> checkNodes(Node object, Node player) {
	//cout << "Astar: 1\n";
	vector<Node> empty;
	//Node zeroNode;
	//zeroNode.pos = vec3(0, 0, 0);

	//bool isV = isValid(player.pos);
	//cout << std::boolalpha;
	//cout << "skunk pos is valid? " << isV << "\n";
	if (!isValid(player.pos)) { //player is unreachable and is in an obstacle
		cerr << "Player is in obstacle!\n";
		return empty;
	}

	if (isDestination(object.pos, player)) { 
		//cout << "Reached destination! You were already there :)\n";
		return empty;
	}

	//cout << "Astar: 1.2\n";

	bool visitedList[MAP_SIZE][MAP_SIZE];

	//initialize map array to be filled in later
	array<array<Node, MAP_SIZE>, MAP_SIZE> map;
	for (int x=0; x<MAP_SIZE; x++) {
		for (int z=0; z<MAP_SIZE; z++) {
			map[x][z].fCost = FLT_MAX;
			map[x][z].gCost = FLT_MAX;
			map[x][z].hCost = FLT_MAX;
			map[x][z].parentPos.x = -1;
			map[x][z].parentPos.z = -1;
			map[x][z].pos.x = x;
			map[x][z].pos.z = z;
			visitedList[x][z] = false;
		}
	}


	//init starting list
	int x = object.pos.x;
	int z = object.pos.z;
	map[x][z].fCost = 0.0;
	map[x][z].gCost = 0.0;
	map[x][z].hCost = 0.0;
	map[x][z].parentPos.x = x;
	map[x][z].parentPos.z = z;
	//cout << "wolf pos: " << x << " " << z << "\n";

	set<Node> openList;
	openList.insert(map[x][z]);
	
	bool destinationFound = false;
	while (!openList.empty() && openList.size() < MAP_SIZE*MAP_SIZE) {
		
		float temp = FLT_MAX;
		Node node;
		//cerr << "OpenList:\n";

		//if (openList.size())
		for (Node n : openList) {
			//cerr << "Node: " << n.pos.x << " " << n.pos.z << "n.fCost: "<< n.fCost << "\n";
			if (n.fCost < temp) {
				temp = n.fCost;
				node = n;
			}
		}
		openList.erase(node);
		//cerr << "OpenList after delete\n";
		for (Node n : openList) {
			//cerr << "    Node: " << n.pos.x << " " << n.pos.z << "\n";
		}
		assert(node.pos.x <= MAP_SIZE && node.pos.z <= MAP_SIZE);
		assert(node.pos.z >= 0 || node.pos.x >= 0);
		assert(node.fCost < 10000);

		x = node.pos.x;
		z = node.pos.z;
		//cout << "Selected Node x: " << x << " Node z: " << z << "\n";
		visitedList[x][z] = true;

		if (isDestination(node.pos, player)) {
			destinationFound = true;
			//cerr << "FOUND PLAYER\n";
			return makePath(map, player);
		}

		//loop over all neighboring tiles
		for (int newX=-1; newX <= 1;  newX++) {
			for (int newZ = -1; newZ<=1; newZ++){
				double gNew, hNew, fNew;
				//cout << "Checking neighboring tile: " << x+newX << " " << z+newZ <<"\n";
				if (visitedList[x+newX][z+newZ] == false && isValid(vec3(x + newX, 0, z + newZ))) { //not blocked and unvisited tile
					//cout << "...it is valid\n";
					//calc new costs
					gNew = node.gCost + 1.0;
					hNew = calcH(vec3(x+newX, 0, z+newZ), player);
					fNew = gNew + hNew;
					// cerr << "fNew :                      "<<fNew<<"\n";
					// cerr << "fCost new tile["<<x+newX<<"]["<<z+newZ<<"]:     " << map[x+newX][z+newZ].fCost<<"\n";
					// cerr << "fcost current tile["<<x<<"]["<<z<<"]: " << map[x][z].fCost<<"\n";
					//compare costs to current path
					if (map[x+newX][z+newZ].fCost >= 10000) { //not on openList
						map[x+newX][z+newZ].fCost = fNew;
						map[x+newX][z+newZ].gCost = gNew;
						map[x+newX][z+newZ].hCost = hNew;
						map[x+newX][z+newZ].parentPos.x = x;
						map[x+newX][z+newZ].parentPos.z = z;
						openList.insert(map[x+newX][z+newZ]);
						//cerr << "adding tile " << x+newX << " " << z + newZ << " to open List\n";
					}
					else if (map[x][z].gCost > gNew) { //already on openList
						map[x+newX][z+newZ].parentPos.x = x;
						map[x+newX][z+newZ].parentPos.z = z;
						map[x+newX][z+newZ].gCost = node.gCost + 1.0;
						map[x+newX][z+newZ].hCost = calcH(vec3(x+newX, 0, z+newZ), player);
						map[x+newX][z+newZ].fCost = gNew + hNew;
						//cerr << "Tile is already in openList\n";

					}
				}
			}
		}

		// cout << "MAP:\n";
		// for (int i=0; i<MAP_SIZE; i++){
		// 	for (int j = 0; j<MAP_SIZE; j++) {
		// 		cout<< map[i][j].fCost << " ";
		// 	}
		// 	cout << "\n";
		// }

	}
	if (destinationFound == false) {
			//cout << "Did not find player: " << openList.size() << " nodes left to search\n";
			return empty;
	}
}

bool vecIsLessThanOrEqual( vec3 a, vec3 b) {
	return euclideanDist(a, vec3(0, 0, 0)) <= euclideanDist(b, vec3(0, 0, 0));
}

bool vecIsGreaterThanOrEqual( vec3 a, vec3 b) {
	return euclideanDist(a, vec3(0, 0, 0)) > euclideanDist(b, vec3(0, 0, 0));
}

vec3 Astar::findNextPos(Player p, Transform* tr, shared_ptr<CollisionSys> collSys) {
	collisionSysAstar = collSys;

	Node player;
	player.pos = vec3(round(p.pos)) + vec3(MAP_SIZE/2, 0, MAP_SIZE/2); //convert from world coors to map coords
	if (player.pos.y > 0) {
		//cerr << "Player above ground and unreachable";
		return tr->pos;
	}
	player.pos = vec3(player.pos.x, 0, player.pos.z);

	Node object;
	object.pos = tr->pos + vec3(MAP_SIZE/2, 0, MAP_SIZE/2);
	//cout << "Player POS " << player.pos.x << " " << player.pos.y << " " << player.pos.z << "\n";
	//cout << "Wolf POS " << object.pos.x << " " << object.pos.y << " " << object.pos.z << "\n";
	vector<Node> moves;

	assert(!(object.pos.x > MAP_SIZE || object.pos.z > MAP_SIZE));
	assert(!(object.pos.z < 0 || object.pos.x < 0));

	moves = checkNodes(object, player);

	if (!moves.empty()){
		glm::vec3 retMove = vec3(moves.front().pos.x-MAP_SIZE/2, 0, moves.front().pos.z-MAP_SIZE/2); //convert map coords back to world coords
		glm::vec3 trPos = tr->pos;
		//if (retMove == tr->pos) {
		// if (vecIsLessThanOrEqual(retMove,  trPos + vec3(0.5f)) && vecIsLessThanOrEqual(retMove, trPos + vec3(0.5f)) &&
		// 	vecIsGreaterThanOrEqual(retMove, trPos - vec3(0.5f)) && vecIsGreaterThanOrEqual(retMove, trPos + vec3(0.5f)) ) 
		// 	{
			if (moves.size() > 1) { //retMove is same as pos, so return next pos in moveslist
				moves.erase(moves.begin());
				return vec3(moves.front().pos.x-MAP_SIZE/2, 0, moves.front().pos.z-MAP_SIZE/2);
			}
			return tr->pos;
		//}
		//return retMove;
	}

	return tr->pos;
}


