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

class DamageSys : public System
{
public:
	float POISON_TICK_TIME;
	void init(float enemyPoisonTimer);
	void update(vector<Entity>* trail, float frameTime);
private:
	void simulatePoisonCollision(Entity entity, float frameTime);
};