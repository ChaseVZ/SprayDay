#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Shape.h"
#include "Texture.h"

using namespace glm;
using namespace std;

struct ShapeGroup {
	vector<shared_ptr<Shape>> shapes;
	vector<shared_ptr<Texture>> textures;
};