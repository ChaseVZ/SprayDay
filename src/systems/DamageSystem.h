#pragma once 

#include "../Enemy.h"
#include "../DamageComponent.h"
#include "../MatrixStack.h"
#include "../ShapeGroup.h"
#include "../EcsCore/EcsTypes.h"
#include "../EcsCore/Coordinator.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;


namespace DamageSystem{
    void update(vector<DamageComponent>* damageComps, vector<Enemy>* enemies, vector<Entity>* trail, float frametime);
}