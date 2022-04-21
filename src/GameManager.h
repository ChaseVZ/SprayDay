#ifndef GM_H
#define GM_H

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace glm;



const int MAP_SIZE = 200; // world bounds = -MAP_SIZE/2 to +MAP_SIZE/2

class GameManager
{
private:
	// instance
	static GameManager* instance;

	// data
	int colMap[MAP_SIZE][MAP_SIZE] = {0};


	// private functions
	void setColMap() {
		for (int i = 0; i < MAP_SIZE; i++) {
			for (int j = 0; j < MAP_SIZE; j++) {
				std::cout << colMap[i][j];
			}
		}
	}

	// private constructor
	GameManager() { }
public:
	static GameManager* GetInstance()
	{
		if (!instance) { instance = new GameManager(); }
		return instance;
	}

	int getSize() { return MAP_SIZE; }
};

//GameManager* GameManager::instance = NULL;

#endif