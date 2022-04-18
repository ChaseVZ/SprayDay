#pragma once
#include "ShapeGroup.h"

namespace initShapes {
	ShapeGroup load(string obj_dir, string mtl_dir, string tex_dir, bool textured, bool reverse_norms, int* tex_idx);
}