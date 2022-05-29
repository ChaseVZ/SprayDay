#include "PathingSystem.h"
#include "../Enemy.h"
#include "../Player.h"
#include "../Program.h"
#include "../GameManager.h"
#include "../MatrixStack.h"
#include "../MatrixStack.h"


#include <iostream>
#include <list>
#include <glad/glad.h>

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

float PLAYER_DAMAGE_CAP = 55.0; // maximum damage per second (player has 100 hp)

using namespace glm;
using namespace std;

extern Coordinator gCoordinator;

    vec3 faceAway(vec3 p1, vec3 p2) {
		return normalize(vec3(p1.x - p2.x, 0.0, p1.z - p2.z)* length(p1))*vec3(0.2);
	}

    bool PathingSys::checkCollisionsWithEnemies(Entity currentEnemy) {
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


    bool collideWithPlayer(vec3 nextPos, Player* p, Enemy* e, float frameTime, float* damageFromEnemies) {
        // if (nextPos.x + e->boRad > 125 || nextPos.x - e->boRad < -125)
        // {
        //     e->vel = vec3(-1*(e->vel.x), e->vel.y, e->vel.z);
        //     return true;
        // }
        // if (nextPos.z + e->boRad> 125 || nextPos.z - e->boRad < -125)
        // {
        //     e->vel = vec3(e->vel.x, e->vel.y, -1*(e->vel.z));
        //     return true;
        // }
        if (sqrtf(pow((nextPos.x - p->pos.x), 2) + pow((nextPos.z - p->pos.z), 2)) < e->boRad + p->boRad) 
        {
			*damageFromEnemies += 30.0;
            return true;
        }
        return false;
    }

	bool useOldDest(vec3 a, vec3 b, float epsilon) {
		if (abs(a.x - b.x) <= epsilon && abs(a.y - b.y) <= epsilon && abs(a.z - b.z) <= epsilon) {
			return false;
		}
		return true;
	}
	vec3 getXZ(vec3 inVec) {
		return vec3(inVec.x, 0, inVec.z);
	}
    void move(Player* p, float dt, Enemy* e, Transform* tr, shared_ptr<CollisionSys> collSys,
		float frametime, float* damageFromEnemies) {
       if (!collideWithPlayer(tr->pos, p, e, dt, damageFromEnemies))
       {
			if (!useOldDest(e->nextTile, tr->pos, (e->baseSpeed)*frametime)) {
				e->nextTile = Astar::findNextPos(*p, tr, collSys);
				
			}
			e->prevVel = e->vel;
			e->vel = (e->nextTile +vec3(0.5, 0.0, 0.5) - tr->pos);

			//cerr << "Moved Wolf to tile vec3(" << nextPos.x << " " << nextPos.y << " " << nextPos.z << ")\n";
			//cerr << "Moved Wolf by vec3(" << e->vel.x << " " << e->vel.y << " " << e->vel.z << ")\n";
			vec3 velXZ = getXZ(e->vel);
			vec3 prevVelXZ = getXZ(e->prevVel);
			if (e->vel != vec3(0)) {
				e->vel = normalize(e->vel)*vec3(e->baseSpeed) / vec3(4.0f) ;
				if (velXZ+ prevVelXZ == vec3(0))
					tr->lookDir = velXZ;
				else
					tr->lookDir = velXZ + prevVelXZ;
			}
			tr->pos += e->vel*dt;
       }
    }


void applyDamageToPlayer(Player* p,  float damageFromEnemies, float frameTime, bool* gameOver) {
	float rawDamageToPlayer = (std::min)(damageFromEnemies, PLAYER_DAMAGE_CAP);
	p->health -= rawDamageToPlayer * frameTime;
	p->health = (std::max)(p->health, 0.0f);
	if (p->health == 0.0) {
		*gameOver = true;
		//cout << "YOU LOSE :(" << endl;
		//exit(EXIT_SUCCESS);
	}
}

void PathingSys::update(float frameTime, Player* player, shared_ptr<CollisionSys> collSys, bool* gameOver) {
	/*
	for (Entity const& entity : mEntities) {
		this->checkCollisionsWithEnemies(entity);
	}*/

	float damageFromEnemies = 0.0;
	for (Entity const& entity : mEntities) {
		Enemy& entityEnemyComp = gCoordinator.GetComponent<Enemy>(entity);
		Transform& entityTransComp = gCoordinator.GetComponent<Transform>(entity);

		move(player, frameTime*50, &entityEnemyComp, &entityTransComp, collSys, entity, &damageFromEnemies);
	}
	
	applyDamageToPlayer(player, damageFromEnemies, frameTime, gameOver);
	
}

void PathingSys::init() {
}

