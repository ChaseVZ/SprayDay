#include "DamageSystem.h"
#include <iostream>

const int SPRAY_HITBOX_FACTOR = 1; // increase for better performance

extern Coordinator gCoordinator;

bool hpLessThanOne(Entity entity) {
	DamageComponent& damageComp = gCoordinator.GetComponent<DamageComponent>(entity);
	return damageComp.currentHp <= 0;
}
void DamageSys::simulatePoisonCollision(Entity entity, float frameTime) {
	DamageComponent& enemyDC = gCoordinator.GetComponent<DamageComponent>(entity);
	AnimationComponent& enemyAC = gCoordinator.GetComponent<AnimationComponent>(entity);
	enemyDC.poisonTimer -= frameTime;
	if (enemyDC.poisonTimer < 0) {
		float excessTime = -enemyDC.poisonTimer;
		enemyDC.currentHp -= POISON_TICK_TIME;
		enemyDC.poisonTimer = POISON_TICK_TIME - excessTime;
		enemyAC.poisonDamageFrame = 1;
	}
}

bool checkPoisonCollision(Transform enemyTr, Enemy enemy, vector<Entity>* trail) {
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

void animatePoison(Entity entity) {
	AnimationComponent& enemyAC = gCoordinator.GetComponent<AnimationComponent>(entity);
	Transform& enemyTr = gCoordinator.GetComponent<Transform>(entity);
	int pFrame = enemyAC.poisonDamageFrame;
	if (pFrame > 0) {
		enemyTr.scale = vec3(5.3);
		enemyAC.poisonDamageFrame += 1;
	}
	if (pFrame > 4) {
		enemyAC.poisonDamageFrame = 0;
		enemyTr.scale = vec3(5.0);
	}
}
void DamageSys :: update(vector<Entity>* trail, float frameTime)
{
	set<Entity>::iterator itr;
	for (itr = mEntities.begin(); itr != mEntities.end(); ){
		Entity entity = *itr;
		itr++; // so that iterator is not messed up by deletion
		Transform & enemyTr = gCoordinator.GetComponent<Transform>(entity);
		RenderComponent& enemyRC = gCoordinator.GetComponent<RenderComponent>(entity);
		AnimationComponent& enemyAC = gCoordinator.GetComponent<AnimationComponent>(entity);
		Enemy & enemy = gCoordinator.GetComponent<Enemy>(entity);
		
		if (checkPoisonCollision(enemyTr, enemy, trail)) {
			simulatePoisonCollision(entity, frameTime);
			set<Entity>::iterator currentEnt = itr;
			enemyAC.inPoison = true;
		}
		else {
			enemyAC.inPoison = false;
		}
		animatePoison(entity);
		if (hpLessThanOne(entity)) {
			gCoordinator.DestroyEntity(entity);
		}
	}	
}
void DamageSys::init(float poisonTickTime){
	POISON_TICK_TIME = poisonTickTime;
}
