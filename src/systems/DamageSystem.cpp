#include "DamageSystem.h"
#include <iostream>
const int SPRAY_HITBOX_FACTOR = 1; // increase for better performance 

extern Coordinator gCoordinator;

bool hpNegative(Entity entity) {
	DamageComponent& damageComp = gCoordinator.GetComponent<DamageComponent>(entity);
	return damageComp.currentHp < 0;
}

bool checkCollision(Transform enemyTr, Enemy enemy, vector<Entity>* trail) {
	for (int j = 0; j < trail->size(); j += SPRAY_HITBOX_FACTOR) {
		RenderComponent sprayRC = gCoordinator.GetComponent<RenderComponent>((*trail)[j]);
		Transform sprayTR = gCoordinator.GetComponent<Transform>((*trail)[j]);
		vec3 trailPos = sprayTR.pos;
		vec3 enemyPos = enemyTr.pos;
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
void DamageSys :: update(vector<Entity>* trail, float frameTime)
{
	set<Entity>::iterator itr;
	for (itr = mEntities.begin(); itr != mEntities.end(); ){
		Entity entity = *itr;
		itr++; // so that iterator is not messed up by deletion
		Transform & enemyTr = gCoordinator.GetComponent<Transform>(entity);
		RenderComponent& enemyRC = gCoordinator.GetComponent<RenderComponent>(entity);
		Enemy & enemy = gCoordinator.GetComponent<Enemy>(entity);
		DamageComponent& enemyDC = gCoordinator.GetComponent<DamageComponent>(entity);
		if (checkCollision(enemyTr, enemy, trail)) {
			enemyDC.currentHp-= 5*frameTime;
			set<Entity>::iterator currentEnt = itr;
			if (hpNegative(entity)) {
				gCoordinator.DestroyEntity(entity);
			}
		}
	}	
}

void DamageSys::init(){
}
