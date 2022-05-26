#pragma once
#include "ShapeGroup.h"

namespace initShapes {
	ShapeGroup load(string obj_dir, string mtl_dir, string tex_dir, bool textured, bool reverse_norms, int* tex_idx, string fbx_dir);
	ShapeGroup load(string obj_dir, string mtl_dir, string tex_dir, bool textured, bool reverse_norms, int* tex_idx);
	ShapeGroup load(string obj_dir, shared_ptr<Texture> tex, bool reverse_norms, int* tex_idx);
}