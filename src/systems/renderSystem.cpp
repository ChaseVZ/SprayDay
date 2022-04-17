#include "RenderSystem.h"
#include "../Program.h"
#include <iostream>

namespace RenderSystem {
	void draw(ShapeGroup sg, shared_ptr<Program> curS)
	{
		for (int i = 0; i < sg.shapes.size(); i++) {
			sg.textures[i]->bind(curS->getUniform("Texture0"));
			sg.shapes[i]->draw(curS);
		}
	}
}