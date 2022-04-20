#pragma once

#include "Shape.h"
#include "Texture.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

struct ShapeGroup {
	vector<shared_ptr<Shape>> shapes;
	vector<shared_ptr<Texture>> textures;
};

struct RenderComponent {
	ShapeGroup* sg;
	vec3 pos;
	mat4 lookMat;
	float scale;
	float transparency;
};