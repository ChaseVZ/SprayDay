//#include "GameManager.h"
//
//GameManager* GameManager::instance = NULL;
//
//// covert world coordinate to collision map index
//int worldToMap(float val)
//{
//	/* val / TileSize; then truncate to an int */
//	int s = GameManager::GetInstance()->GameManager::getSize();
//	int res = static_cast<int> (val) + (s / 2);
//
//	// Error checks
//	if (res < 0) { cout << "ERROR: Negative collision index: " << val << endl; return -1; }
//	if (res > s) { cout << "ERROR: Overload collision index: " << val << endl; return -1; }
//	if (s % 2 == 1) { cout << "ERROR: Map size is not even!!" << endl; }
//
//	return res;
//}
//
//void GameManager::verifyCollisionAddition(int i, int j, CollisionT c)
//{
//	if (i >= 0 && j >= 0 && i < MAP_SIZE && j < MAP_SIZE) { colMap[i][j] = c; }
//}
//
//// LOGIC IS FOR CRATES, TODO: make uniform for all types
//void GameManager::addCollision(vec3 pos, CollisionT c)
//{
//	//if (c != CRATE) { return; } // only do CRATE collisions for now
//
//	int i = worldToMap(pos.x);
//	int j = worldToMap(pos.z);
//
//	if (i < 0 || j < 0) { return; } // error checks
//
//	if (i != 0 && j != 0) {
//		verifyCollisionAddition(i - 1, j - 1, c);
//		verifyCollisionAddition(i - 0, j - 1, c);
//		verifyCollisionAddition(i + 1, j - 1, c);
//		verifyCollisionAddition(i - 1, j + 0, c);
//		verifyCollisionAddition(i - 1, j + 1, c);
//	}
//
//	verifyCollisionAddition(i + 1, j + 0, c);
//	verifyCollisionAddition(i + 0, j + 1, c);
//	verifyCollisionAddition(i + 0, j + 0, c);
//	verifyCollisionAddition(i + 1, j + 1, c);
//}
//
//
//bool GameManager::isCollision(int i, int j)
//{
//	if (i < 0 || j < 0) { return true; } // don't let anything move outside of mapped world
//	if (i >= MAP_SIZE || j >= MAP_SIZE) { return true; } // don't let anything move outside of mapped world
//	if (colMap[i][j] == 1) { return true; } // check map
//	return false;
//
//}
//
//bool GameManager::checkCollide(vec3 pos, float radius)
//{
//	int size = 0;
//
//	// top right corner
//	//cout << "radius " << radius << " " << pos.x << " " << pos.z << endl;
//	
//	int i1 = worldToMap(pos.x + radius);
//	int j1 = worldToMap(pos.z + radius);
//	if (isCollision(i1, j1)) { return true; }
//
//	// bot left corner
//	int i3 = worldToMap(pos.x - radius);
//	int j3 = worldToMap(pos.z - radius);
//	if (isCollision(i3, j3)) { return true; }
//
//	// bot right corner
//	int i2 = (i1 + i3 + j1 - j3) / 2;
//	int j2 = (i3 - i1 + j1 + j3) / 2;
//	if (isCollision(i2, j2)) { return true; }
//
//	// top left corner
//	int i4 = (i1 + i3 + j3 - j1) / 2;
//	int j4 = (i1 - i3 + j1 + j3) / 2;
//	if (isCollision(i4, j4)) { return true; }
//
//	// tiles along vertical edges 
//	//cout << "j4: " << j4 << " j3: " << j3 << " k: " << abs(j4 - j3) - 2 << endl;
//	//cout << i1 << " " << i2 << " ||| " << endl << endl;
//	for (int k = 1; k < abs(j4 - j3) - 2 + 1; k++) {
//		if (isCollision(i4, j4 - k)) { return true; } // vertical R
//		if (isCollision(i1, j1 - k)) { return true; } // vetical L
//		if (isCollision(i4 + k, j4)) { return true; } // horizontal T
//		if (isCollision(i3 + k, j3)) { return true; } // horizontal B
//	}
//
//	return false;
//}