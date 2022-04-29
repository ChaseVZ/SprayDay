#pragma once

#include "Shape.h"
#include "Texture.h"
#include "Program.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "CollisionEnum.h"

using namespace std;
using namespace glm;

struct ShapeGroup {
	vector<shared_ptr<Shape>> shapes;
	vector<shared_ptr<Texture>> textures;
};


struct RenderComponent {
	ShapeGroup* sg;
	float transparency;
	shared_ptr<Program> shader;
	int cullDir;
	Collision c;
};