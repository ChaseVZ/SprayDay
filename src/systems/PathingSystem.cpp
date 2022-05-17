#include "PathingSystem.h"
#include "../Enemy.h"
#include "../Player.h"
#include "../Program.h"
#include "../GameManager.h"
#include "../MatrixStack.h"


#include <iostream>
#include <list>
#include <glad/glad.h>

#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

using namespace glm;
using namespace std;

extern Coordinator gCoordinator;

    vec3 faceAway(vec3 p1, vec3 p2) {
		return normalize(vec3(p1.x - p2.x, 0.0, p1.z - p2.z)* length(p1))*vec3(0.2);
	}

    // vec3 calcScareVel(vec3 ePos, vec3 pPos) {
	// 	return normalize(vec3(ePos.x - pPos.x, 0.21, ePos.z - pPos.z));
	// }

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


    bool collideWithPlayer(vec3 nextPos, Player* p, Enemy* e, float frameTime) {
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
            e->exploding = true;
			p->health -= frameTime;
			p->health = std::max(p->health, 0.0f);
			if (p->health == 0.0) {
				//cout << "YOU LOSE :(" << endl;
				//exit(EXIT_SUCCESS);
			}

            return true;
        }
        return false;
    }

	bool vecEpsilonEqual(vec3 a, vec3 b, float epsilon) {
		if (abs(a.x - b.x) <= epsilon && abs(a.y - b.y) <= epsilon && abs(a.z - b.z) <= epsilon) {
			return true;
		}
		return false;
	}

    void move(Player* p, float dt, Enemy* e, Transform* tr, shared_ptr<CollisionSys> collSys) {
       if (!collideWithPlayer(tr->pos, p, e, dt))
       {
			if (vecEpsilonEqual(e->nextTile, tr->pos, e->baseSpeed/4.0f)) {
				e->nextTile = Astar::findNextPos(*p, tr, collSys);
			}
			e->vel = (e->nextTile - tr->pos);

			//cerr << "Moved Wolf to tile vec3(" << nextPos.x << " " << nextPos.y << " " << nextPos.z << ")\n";
			//cerr << "Moved Wolf by vec3(" << e->vel.x << " " << e->vel.y << " " << e->vel.z << ")\n";
			if (e->vel != vec3(0)) {
				e->vel = normalize(e->vel)*vec3(e->baseSpeed) / vec3(4.0f) ;
				tr->lookDir = normalize(e->vel);
			}
			tr->pos += e->vel*dt;
        }
    }

void PathingSys::update(float frametime, Player* player, shared_ptr<CollisionSys> collSys) {

	for (Entity const& entity : mEntities) {
		this->checkCollisionsWithEnemies(entity);
	}
	for (Entity const& entity : mEntities) {
		Enemy& entityEnemyComp = gCoordinator.GetComponent<Enemy>(entity);
		Transform& entityTransComp = gCoordinator.GetComponent<Transform>(entity);

		move(player, frametime*50, &entityEnemyComp, &entityTransComp, collSys);

	}
}

void PathingSys::init() {
}

