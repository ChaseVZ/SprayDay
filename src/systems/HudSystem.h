#ifndef HUD_RENDER_SYS
#define HUD_RENDER_SYS
#include <iostream>
#include "../ShapeGroup.h"
#include "../GLSL.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Program.h"
#include "../MatrixStack.h"
#include "../EcsCore/System.h"
#include "../GameManager.h"
#include "../EcsCore/Coordinator.h"
#include "../Components/HudComponent.h"

class HudSys : public System
{
public:
	void init();
	void update(shared_ptr<MatrixStack> Projection);
};

#endif