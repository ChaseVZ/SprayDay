#pragma once
#define NOMINMAX
#include "../Enemy.h"
#include "../Player.h"
#include "../Program.h"
#include "../MatrixStack.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Components/Transform.h"
#include "CollisionSystem.h"
#include "../Astar.h"
#include "../Enemy.h"
#include "../EcsCore/EcsTypes.h"
#include "../EcsCore/Coordinator.h"
#include "../EcsCore/System.h"
#include <vector>

using namespace std;
using namespace glm;
/*
//Enemy Enemy(vec3 position, vec3 velocity, float boundingRadius);
void move(Player p, float dt, Enemy* e, Transform* tr);

// void explode(Enemy* e);
bool checkCollisions(Entity currentEnemy, set<Entity> enemies);
vec3 faceAway(vec3 p1, vec3 p2);
vec3 calcScareVel(vec3 ePos, vec3 pPos);
*/

class PathingSys : public System
{
public:
    bool collideWithPlayer(vec3 nextPos, Player* p, Enemy* e, float frameTime);
    bool checkCollisionsWithEnemies(Entity currentEnemy);
    void init();

    void update(float frametime, Player* player, shared_ptr<CollisionSys> collSys, bool* isGrey);

};