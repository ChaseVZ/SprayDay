#pragma once 

#include "../Enemy.h"
#include "../DamageComponent.h"
#include "../MatrixStack.h"
#include "../ShapeGroup.h"
#include "../EcsCore/EcsTypes.h"
#include "../EcsCore/Coordinator.h"
#include "../Components/Transform.h"
#include "../Components/AnimationComponent.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;




class SpawnSys : public System
{
public:
	void init(int mapSize, float poisonTickTime, ShapeGroup* wolf, shared_ptr<Program> texProg);
	void update(float frameTime);
	int MAP_SIZE;
	float POISON_TICK_TIME;
private:
	ShapeGroup* wolf;
	ShapeGroup* bear;

	void spawnEnemies(float frametime);
	void initWolf();
	vec3 getRandStart();
	shared_ptr<Program> texProg;
};


