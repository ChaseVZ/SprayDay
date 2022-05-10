#include "PathingSystem.h"
#include "../Enemy.h"
#include "../Player.h"
#include "../Program.h"
#include "../GameManager.h"
#include "../MatrixStack.h"



#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

extern Coordinator gCoordinator;

    vec3 faceAway(vec3 p1, vec3 p2) {
		return normalize(vec3(p1.x - p2.x, 0.0, p1.z - p2.z)* length(p1))*vec3(0.2);
	}

    vec3 calcScareVel(vec3 ePos, vec3 pPos) {
		return normalize(vec3(ePos.x - pPos.x, 0.21, ePos.z - pPos.z));
	}

    bool PathingSys::checkCollisions(Entity currentEnemy) {
		set<Entity>::iterator itr;
		Transform& currentTr = gCoordinator.GetComponent<Transform>(currentEnemy);
		Enemy& currentEneComp = gCoordinator.GetComponent<Enemy>(currentEnemy);
		for (itr = mEntities.begin(); itr != mEntities.end(); itr++){
			Entity otherEnemy = *itr;
			if (otherEnemy != currentEnemy) {
				Transform& otherTr = gCoordinator.GetComponent<Transform>(otherEnemy);
				Enemy& otherEneComp = gCoordinator.GetComponent<Enemy>(otherEnemy);
				if (length(vec3(currentTr.pos - otherTr.pos)) < currentEneComp.boRad*2) {
					currentEneComp.vel = faceAway(currentTr.pos, otherTr.pos);
					//enemies[sID].vel = vec3(0, 0.8, 0);
					return true;
				}
			}
		}
		return false;
	}

    bool collide(vec3 nextPos, Player* p, Enemy* e, float frameTime) {
        if (nextPos.x + e->boRad > 125 || nextPos.x - e->boRad < -125)
        {
            e->vel = vec3(-1*(e->vel.x), e->vel.y, e->vel.z);
            return true;
        }
        if (nextPos.z + e->boRad> 125 || nextPos.z - e->boRad < -125)
        {
            e->vel = vec3(e->vel.x, e->vel.y, -1*(e->vel.z));
            return true;
        }

        if (sqrtf(pow((nextPos.x - p->pos.x), 2) + pow((nextPos.z - p->pos.z), 2)) < e->boRad + p->boRad) 
        {
            e->exploding = true;
			p->health -= frameTime;
            return true;
        }
        return false;
    }

    void move(Player* p, float frameTime, Enemy* e, Transform* tr) {
        if (!collide(tr->pos + e->vel*frameTime, p, e, frameTime))
        {
            tr->pos += e->vel*frameTime;
			tr->lookDir = normalize(p->pos - tr->pos);
			float mag = glm::length(e->vel);
			e->vel = tr->lookDir * mag;
        }
    }

void PathingSys::update(float frameTime, Player* player) {

	for (Entity const& entity : mEntities) {
		this->checkCollisions(entity);
	}
	for (Entity const& entity : mEntities) {
		Enemy& entityEnemyComp = gCoordinator.GetComponent<Enemy>(entity);
		Transform& entityTransComp = gCoordinator.GetComponent<Transform>(entity);
		move(player, frameTime*50, &entityEnemyComp, &entityTransComp);
	}
}

void PathingSys::init() {
}

