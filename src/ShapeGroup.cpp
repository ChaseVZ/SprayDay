#include "ShapeGroup.h"
#include "Shape.h"
#include "Program.h"

/*
	dir's refer to where shape specific files are located in reference to build folder
	textured: does the shape use textures?
	reverse_norms: if normals need to be flipped (usually false)
	tex_idx: reference to global texture count
*/
//ShapeGroup::ShapeGroup()
//{
//	obj_dir = _obj_dir;
//	mtl_dir = _mtl_dir;
//	tex_dir = _tex_dir;
//	textured = _textured;
//	reverse_norms = _reverse_norms;
//	tex_idx = _tex_idx;
//}
//
//void ShapeGroup::draw(shared_ptr<Program> curS) {
//	for (int i = 0; i < num_mats; i++) {
//		textures[i]->bind(curS->getUniform("Texture0"));
//		shapes[i]->draw(curS);
//	}
//}



