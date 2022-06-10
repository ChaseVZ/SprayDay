#include "PathingSystem.h"

float PLAYER_DAMAGE_CAP = 110.0; // maximum damage per second (player has 100 hp)
float ENEMY_MVMT_INTO_PLAYER = 0.5;

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
	float getEuclidianDist(vec2 a, vec2 b) {
		return sqrtf(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
	}
	float getXZEuclidianDist(vec3 a, vec3 b) {
		return sqrtf(pow((a.x - b.x), 2) + pow((a.z - b.z), 2));
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
		if (abs(p->pos.y - nextPos.y) < 2.0 && getXZEuclidianDist(nextPos, p->pos) < (e->boRad + p->boRad))
		{
			*damageFromEnemies = (*damageFromEnemies + PLAYER_DAMAGE_CAP) / 2.5;
			return getXZEuclidianDist(nextPos, p->pos) < e->boRad + p->boRad - ENEMY_MVMT_INTO_PLAYER;
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
		float frametime, float* damageFromEnemies,
		array<array<array<vec3, IDX_SIZE>, IDX_SIZE>, 2>* simpleStoredMoves,
		array<array<array<vec3, IDX_SIZE>, IDX_SIZE>, 2>* flankStoredMoves) {
       if (!collideWithPlayer(tr->pos, p, e, dt, damageFromEnemies))
       {
			if (!useOldDest(e->nextTile, tr->pos, (e->baseSpeed)*frametime)) {
				e->nextTile = Astar::findNextPos(*p, tr, collSys , e->pathingType, simpleStoredMoves, flankStoredMoves);
			}
			e->vel = (e->nextTile +vec3(0.5, 0.0, 0.5) - tr->pos);
			vec3 velXZ = getXZ(e->vel);
			vec3 prevLookXZ = getXZ(tr->lookDir);
			if (e->vel != vec3(0)) {
				e->vel = normalize(e->vel)*vec3(e->baseSpeed)*vec3(e->debufSpeed) / vec3(4.0f);

				vec3 nextLookDir = velXZ * vec3(0.3) + prevLookXZ * vec3(0.7);
				if (nextLookDir != vec3(0))
					tr->lookDir = nextLookDir;	
				else
					tr->lookDir = velXZ;
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
	}
}

void PathingSys::update(float frameTime, Player* player, shared_ptr<CollisionSys> collSys, bool* gameOver) {
	/*
	for (Entity const& entity : mEntities) {
		this->checkCollisionsWithEnemies(entity);
	}*/

	float damageFromEnemies = 0.0;

	array<array<array<vec3, IDX_SIZE>, IDX_SIZE>, 2>* simpleStoredMoves = new array<array<array<vec3, IDX_SIZE>, IDX_SIZE>, 2>;
	for (int x = 0; x < IDX_SIZE; x++) {
		for (int z = 0; z < IDX_SIZE; z++) {
			for (int y = 0; y < 2; y++) {
				(*simpleStoredMoves)[y][x][z] = vec3(-1);
			}
		}
	}

	array<array<array<vec3, IDX_SIZE>, IDX_SIZE>, 2>* flankStoredMoves = new array<array<array<vec3, IDX_SIZE>, IDX_SIZE>, 2>;
	for (int x = 0; x < IDX_SIZE; x++) {
		for (int z = 0; z < IDX_SIZE; z++) {
			for (int y = 0; y < 2; y++) {
				(*flankStoredMoves)[y][x][z] = vec3(-1);
			}
		}
	}
	for (Entity const& entity : mEntities) {
		Enemy& entityEnemyComp = gCoordinator.GetComponent<Enemy>(entity);
		Transform& entityTransComp = gCoordinator.GetComponent<Transform>(entity);
		move(player, frameTime*50, &entityEnemyComp, &entityTransComp, collSys, entity, &damageFromEnemies, simpleStoredMoves, flankStoredMoves);
	}
	delete simpleStoredMoves;
	delete flankStoredMoves;
	applyDamageToPlayer(player, damageFromEnemies, frameTime, gameOver);
	
}

void PathingSys::init() {
}

