#pragma once
#include "../EcsCore/System.h"
//#include "../animationUtil.h"
#include "../Components/AnimationComponent.h"
#include "../EcsCore/Coordinator.h"
//#define STB_IMAGE_IMPLEMENTATION

using namespace std;
using namespace glm;

class AnimationSys : public System {
public:
	void init();
	//void update(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, shared_ptr<Texture> tex, float elapsedTime, SkeletalComponent sc);
private:

};