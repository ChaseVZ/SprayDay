#ifndef GM_H
#define GM_H

#include "ShapeGroup.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace glm;
using namespace std;


const int MAP_SIZE = 160; // world bounds = -MAP_SIZE/2 to +MAP_SIZE/2
const int TILE_SIZE = 2; // crates take up a 1x1 * crate_size area in world space


class GameManager
{
private:
	// instance
	static GameManager* instance;

	// data
	int colMap[MAP_SIZE + 1][MAP_SIZE + 1] = {};
	vec3 lightPos = vec3(0, 10, 0);

	// private functions
	void setupColMap()
	{
		for (int i = 0; i < MAP_SIZE; i++) {
			for (int j = 0; j < MAP_SIZE; j++) {
				colMap[i][j] = 0;
			}
		}
	}

	bool isCollision(int i, int j);
	void verifyCollisionAddition(int i, int j, Collision c);

	// private constructor
	GameManager() { setupColMap(); }
public:
	static GameManager* GetInstance()
	{
		if (!instance) { instance = new GameManager(); }
		return instance;
	}

	int getSize() { return MAP_SIZE; }
	int getTileSize() { return TILE_SIZE; }
	vec3 getLightPos() { return lightPos; }

	void addCollision(vec3 pos, Collision c); // where c is of type Collision
	bool checkCollide(vec3 pos, float radius);



};

//GameManager* GameManager::instance = NULL;

#endif