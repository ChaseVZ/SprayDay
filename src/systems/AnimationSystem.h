#pragma once
#include "../EcsCore/System.h"
//#include "../animationUtil.h"
#include "../Components/AnimationComponent.h"
#include "../EcsCore/Coordinator.h"
#include "../ShapeGroup.h"
//#define STB_IMAGE_IMPLEMENTATION

using namespace std;
using namespace glm;

class AnimationSys : public System {
public:
	void init(ShapeGroup* wolf, ShapeGroup* bear);
	//void update(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, shared_ptr<Texture> tex, float elapsedTime, SkeletalComponent sc);
private:

};