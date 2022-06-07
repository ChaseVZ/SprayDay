#pragma once 
#include "../EcsCore/EcsTypes.h"
#include "../ShapeGroup.h"
#include "../Components/Transform.h"
#include "../EcsCore/Coordinator.h"
#include "../particleGen.h"
#include "../Components/ParticleComponent.h"

const int PARTICLES_PER_SPRAY = 10;
const int MAX_SPRAY_SPHERES = 100;

class SpraySys : public System
{
public:
	void init(ShapeGroup* spherePtr, shared_ptr<Program>, particleGen* pg);
	void update(float frameTime, vector<Entity>*, int mvmType, vec3 playerPos, vec3 lookDir);

private:
	ShapeGroup* sphere;
	shared_ptr<Program> texProg;
	void generateSpray(vector<Entity>* trail, vec3 playerPos, vec3 lookDir);
};
