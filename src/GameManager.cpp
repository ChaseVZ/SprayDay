#include "GameManager.h"

GameManager* GameManager::instance = NULL;

// covert world coordinate to collision map index
int worldToMap(float val)
{
	/* val / TileSize; then truncate to an int */
	return static_cast<int> ( val / static_cast<float>(GameManager::GetInstance()->getTileSize()));
}


void GameManager::addCollision(vec3 pos)
{
	int i = worldToMap(pos.x);
	int j = worldToMap(pos.z);

	//cout << i << " " << j << " is a collision" << endl;
	colMap[i][j] = 1;
	colMap[i + 1][j] = 1;
	colMap[i][j + 1] = 1;
	colMap[i + 1][j + 1] = 1;

}

bool GameManager::checkCollide(vec3 pos)
{
	int i = worldToMap(pos.x);
	int j = worldToMap(pos.z);

	if (colMap[i][j] == 1) { return true; }
	return false;
}