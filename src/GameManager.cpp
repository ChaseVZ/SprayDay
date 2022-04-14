#include "GameManager.h"

GameManager* GameManager::instance = nullptr;;

GameManager* GameManager::GetInstance()
{
	if (!instance) { instance = new GameManager; }
	return instance;
}
bool GameManager::getCollision() { return collision; }

void GameManager::setCollision(bool setter) {collision = setter;}