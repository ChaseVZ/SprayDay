#ifndef GM_H
#define GM_H

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace glm;
class GameManager
{
private:
	// instance
	static GameManager* instance;

	// data
	int** colMap;
	int MAP_SIZE;

	// private functions
	void setColMap() {
		colMap[MAP_SIZE][MAP_SIZE] = { 0 };

		for (int i = 0; i < MAP_SIZE; i++) {
			for (int j = 0; j < MAP_SIZE; j++) {
				std::cout << colMap[i][j];
			}
		}
	}

	// private constructor
	GameManager() { 
		setColMap(); 
	}

public:
	GameManager* GetInstance()
	{
		if (!instance) { instance = new GameManager(); }
		return instance;
	}

	void setSize(int _size) { MAP_SIZE = _size; }

};

#endif