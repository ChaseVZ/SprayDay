#pragma once 

#include "../Enemy.h"
#include "../DamageComponent.h"
#include "../MatrixStack.h"
#include "../ShapeGroup.h"
#include "../EcsCore/EcsTypes.h"
#include "../EcsCore/Coordinator.h"
#include "../Components/Transform.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

class DamageSys : public System
{
public:
	void init();
	void update(vector<Entity>* trail, float frameTime);
};