#ifndef GM_H
#define GM_H

#include <glm/gtc/type_ptr.hpp>

using namespace glm;
class GameManager
{
	static GameManager* instance;
	bool collision;

	GameManager() { collision = false; }

public:
	static GameManager* GetInstance();
	bool getCollision();
	void setCollision(bool setter);
};

#endif