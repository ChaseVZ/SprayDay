#pragma once 
#include "../EcsCore/EcsTypes.h"
#include "../ShapeGroup.h"
#include "../Components/Transform.h"
#include "../EcsCore/Coordinator.h"
#include "../particleSys.h"
#include "../GLSL.h"



class SpraySys : public System
{
public:
	void init(ShapeGroup* spherePtr, shared_ptr<Program>);
	void update(float frameTime, vector<Entity>*, int mvmType, vec3 playerPos);

private:
	ShapeGroup* sphere;
	shared_ptr<Program> texProg;
	void generateSpray(vector<Entity>* trail, vec3 playerPos);
};
