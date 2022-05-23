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
#include "../Player.h"
#include <ft2build.h>
#include FT_FREETYPE_H
class HudSys : public System
{
public:
	void init(ShapeGroup* cube, std::shared_ptr<Program> cubeProg, unsigned int redTexID,
		const std::string& resourceDirectory, std::shared_ptr<Program> hudProg);
	void update(shared_ptr<MatrixStack> Projection, Player);
};

#endif