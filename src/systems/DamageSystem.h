#pragma once 

#include "../Enemy.h"
#include "../DamageComponent.h"
#include "../MatrixStack.h"
#include "../ShapeGroup.h"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;


namespace DamageSystem{
    void run(vector<DamageComponent>* damageComps, vector<Enemy>* enemies, vector<RenderComponent>* trail, float frametime);
}