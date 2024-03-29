#pragma once
#define NOMINMAX
#include "CollisionSystem.h"
#include "../Enemy.h"
#include "../Player.h"
#include "../Program.h"
#include "../MatrixStack.h"
#include "../GameManager.h"
#include "../Components/Transform.h"
#include "../Astar.h"
#include "../EcsCore/EcsTypes.h"
#include "../EcsCore/Coordinator.h"
#include "../EcsCore/System.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <list>
#include <glad/glad.h>
#include <cmath>
#include <thread>

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