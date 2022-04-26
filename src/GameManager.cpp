#include "GameManager.h"

GameManager* GameManager::instance = NULL;

// covert world coordinate to collision map index
int worldToMap(float val)
{
	/* val / TileSize; then truncate to an int */
	int s = GameManager::GetInstance()->GameManager::getSize();
	int res = static_cast<int> (val) + (s / 2);

	// Error checks
	if (res < 0) { cout << "ERROR: Negative collision index addition" << endl; return -1; }
	if (res >= s) { cout << "ERROR: collision addition index out of bounds" << endl; return -1; }
	if (s % 2 == 1) { cout << "ERROR: Map size is not even!!" << endl; }

	return res;
}


void GameManager::addCollision(vec3 pos)
{
	int i = worldToMap(pos.x);
	int j = worldToMap(pos.z);

	if (i < 0 || j < 0) { return; }

	//cout << i << " " << j << " for" << pos.x << " " << pos.z << endl;
	//cout << i+1 << " " << j << " for" << pos.x << " " << pos.z << endl;
	//cout << i << " " << j+1 << " for" << pos.x << " " << pos.z << endl;
	//cout << i + 1 << " " << j + 1 << " for" << pos.x << " " << pos.z << endl << endl;

	if (i != 0 && j != 0) {
		colMap[i - 1][j - 1] = 1;

		colMap[i + 0][j - 1] = 1;
		colMap[i + 1][j - 1] = 1;

		colMap[i - 1][j + 0] = 1;
		colMap[i - 1][j + 1] = 1;
	}

	colMap[i + 1][j + 0] = 1;
	colMap[i + 0][j + 1] = 1;

	colMap[i + 0][j + 0] = 1;
	colMap[i + 1][j + 1] = 1;
}

bool GameManager::checkCollide(vec3 pos, float radius)
{
	int i = worldToMap(pos.x + radius);
	int j = worldToMap(pos.z + radius);

	if (colMap[i][j] == 1) { cout << "COL:" << i << " " << j << endl; return true; }

	i = worldToMap(pos.x - radius);
	j = worldToMap(pos.z - radius);

	if (colMap[i][j] == 1) { cout << "COL2:" << i << " " << j << endl; return true; }

	i = worldToMap(pos.x + radius);
	j = worldToMap(pos.z - radius);

	if (colMap[i][j] == 1) { cout << "COL3:" << i << " " << j << endl; return true; }

	i = worldToMap(pos.x - radius);
	j = worldToMap(pos.z + radius);

	if (colMap[i][j] == 1) { cout << "COL4:" << i << " " << j << endl; return true; }

	return false;
}