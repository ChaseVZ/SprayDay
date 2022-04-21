#include "DamageSystem.h"
#include <iostream>

namespace DamageSystem {

	void checkHp(vector<DamageComponent>* damageComps, vector<Enemy>* enemies) {
		for (int i = 0; i < enemies->size(); i += 1){
			if ((*damageComps)[i].currentHp < 0) {
				enemies->erase(enemies->begin() + i);
				damageComps->erase(damageComps->begin() + i);
			}
		}
	}

	bool checkCollision(Enemy enemy, vector<RenderComponent>* trail) {
		for (int j = 0; j < trail->size(); j += 2) {
			vec3 trailPos = (*trail)[j].pos;
			vec3 enemyPos = enemy.pos;
			/*
			cout << "enemy pos:" << enemy.pos.x << " " << enemy.pos.z << endl;
			cout << "trail pos:" << trailPos.x << " " << trailPos.z << endl;
			cout << " "<< length(trailPos + enemyPos) << endl;
			*/
			if (length(trailPos - enemyPos) <= enemy.boRad + 1.0) {
				//cout << "enemy hit spray at" << trailPos.x << " " << trailPos.z << "length: " << length(trailPos + enemyPos) << endl;
				return true;
			}
		}
		return false;
	}
	void run(vector<DamageComponent>* damageComps, vector<Enemy>* enemies, vector<RenderComponent>* trail, float frametime)
	{
		for (int i = 0; i < enemies->size(); i += 1){
			if (checkCollision((*enemies)[i], trail)) {
				
				(*damageComps)[i].currentHp-= 5*frametime;
			}
		}

		checkHp(damageComps, enemies);
		
	}
}
