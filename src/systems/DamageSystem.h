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
	shared_ptr<Program> redShader;
	shared_ptr<Program> texShader;
	void init(float enemyPoisonTimer, shared_ptr<Program> redProg, shared_ptr<Program> texProg);
	void update(vector<Entity>* trail, float frameTime, int* enemiesKilled);
private:
	void simulatePoisonCollision(Entity entity, float frameTime);
	void animatePoison(Entity entity);
};