#ifndef ShapeGroup_H
#define ShapeGroup_H

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Shape.h"
#include "Texture.h"

using namespace glm;
using namespace std;

class ShapeGroup
{
	string obj_dir;
	string mtl_dir;
	string tex_dir;

	bool textured;
	bool reverse_norms;

	vector<shared_ptr<Shape>> shapes;
	vector<shared_ptr<Texture>> textures;

	int tex_idx;
	int num_mats; // used to update global count of textures (passed to next ShapeGroup as tex_idx)

	void loadTexturedOBJ();
	void loadNonTexturedOBJ();
	void initTOShapes(bool rc, vector<tinyobj::shape_t> TOshapes);

public:
	ShapeGroup(string _obj_dir, string _mtl_dir, string _tex_dir, bool _textured, bool _reverse_norms, int _tex_idx);
	void draw(shared_ptr<Program> curS);
	void load();

	bool getNumMats() const { return num_mats; }

};

#endif