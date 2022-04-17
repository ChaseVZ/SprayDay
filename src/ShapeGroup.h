#pragma once

#include "Shape.h"
#include "Texture.h"

using namespace std;

struct ShapeGroup {
	vector<shared_ptr<Shape>> shapes;
	vector<shared_ptr<Texture>> textures;
};