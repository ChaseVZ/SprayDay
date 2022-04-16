#include "ShapeGroup.h"
#include "Shape.h"
#include "Program.h"

/*
	dir's refer to where shape specific files are located in reference to build folder
	textured: does the shape use textures?
	reverse_norms: if normals need to be flipped (usually false)
	tex_idx: reference to global texture count
*/
ShapeGroup::ShapeGroup(string _obj_dir, string _mtl_dir, string _tex_dir, bool _textured, bool _reverse_norms, int _tex_idx)
{
	obj_dir = _obj_dir;
	mtl_dir = _mtl_dir;
	tex_dir = _tex_dir;
	textured = _textured;
	reverse_norms = _reverse_norms;
	tex_idx = _tex_idx;
}
void ShapeGroup::load() 
{
	if (textured){ loadTexturedOBJ(); }
	else { loadNonTexturedOBJ(); }
}

void ShapeGroup::draw(shared_ptr<Program> curS) {
	for (int i = 0; i < num_mats; i++) {
		textures[i]->bind(curS->getUniform("Texture0"));
		shapes[i]->draw(curS);
	}
}

void resize_obj(std::vector<tinyobj::shape_t>& shapes) {
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
	float epsilon = 0.001;

	minX = minY = minZ = 1.1754E+38F;
	maxX = maxY = maxZ = -1.1754E+38F;

	//Go through all vertices to determine min and max of each dimension
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			if (shapes[i].mesh.positions[3 * v + 0] < minX) minX = shapes[i].mesh.positions[3 * v + 0];
			if (shapes[i].mesh.positions[3 * v + 0] > maxX) maxX = shapes[i].mesh.positions[3 * v + 0];

			if (shapes[i].mesh.positions[3 * v + 1] < minY) minY = shapes[i].mesh.positions[3 * v + 1];
			if (shapes[i].mesh.positions[3 * v + 1] > maxY) maxY = shapes[i].mesh.positions[3 * v + 1];

			if (shapes[i].mesh.positions[3 * v + 2] < minZ) minZ = shapes[i].mesh.positions[3 * v + 2];
			if (shapes[i].mesh.positions[3 * v + 2] > maxZ) maxZ = shapes[i].mesh.positions[3 * v + 2];
		}
	}

	//From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = maxX - minX;
	yExtent = maxY - minY;
	zExtent = maxZ - minZ;
	if (xExtent >= yExtent && xExtent >= zExtent) {
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent) {
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent) {
		maxExtent = zExtent;
	}
	scaleX = 2.0 / maxExtent;
	shiftX = minX + (xExtent / 2.0);
	scaleY = 2.0 / maxExtent;
	shiftY = minY + (yExtent / 2.0);
	scaleZ = 2.0 / maxExtent;
	shiftZ = minZ + (zExtent) / 2.0;

	//Go through all verticies shift and scale them
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			shapes[i].mesh.positions[3 * v + 0] = (shapes[i].mesh.positions[3 * v + 0] - shiftX) * scaleX;
			assert(shapes[i].mesh.positions[3 * v + 0] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 0] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3 * v + 1] = (shapes[i].mesh.positions[3 * v + 1] - shiftY) * scaleY;
			assert(shapes[i].mesh.positions[3 * v + 1] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 1] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3 * v + 2] = (shapes[i].mesh.positions[3 * v + 2] - shiftZ) * scaleZ;
			assert(shapes[i].mesh.positions[3 * v + 2] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 2] <= 1.0 + epsilon);
		}
	}
}

void ShapeGroup::loadNonTexturedOBJ()
{
	vector<tinyobj::shape_t> TOshapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;
	bool rc;
	rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (obj_dir).c_str());

	resize_obj(TOshapes);
	if (!rc) {
		cerr << errStr << endl;
	}
	else {
		initTOShapes(rc, TOshapes);
	}
}

void ShapeGroup::initTOShapes(bool rc, vector<tinyobj::shape_t> TOshapes)
{
	for (int idx = 0; idx < TOshapes.size(); idx++) {
		shared_ptr<Shape> piece = make_shared<Shape>();
		piece->createShape(TOshapes[idx]);
		piece->calcNorms();
		piece->normalizeNorBuf();
		if (reverse_norms) { piece->reverseNormals(); }
		piece->measure();
		piece->init();
		shapes.push_back(piece);
	}
}

void setTexVector(vector<shared_ptr<Texture>>& mapTexture, const std::string& textureDir, int numTextures, vector<tinyobj::material_t> objMaterials) {
	for (int i = numTextures; i < objMaterials.size() + numTextures; i++) {
		shared_ptr<Texture> mapTextureX = make_shared<Texture>();
		mapTextureX->setFilename(textureDir + (objMaterials[i - numTextures].diffuse_texname));
		mapTextureX->init();
		mapTextureX->setUnit(i);
		mapTextureX->setWrapModes(GL_REPEAT, GL_REPEAT);
		mapTexture.push_back(mapTextureX);
	}
}

void ShapeGroup::loadTexturedOBJ() {
	vector<tinyobj::shape_t> TOshapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;
	bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (obj_dir).c_str(), (mtl_dir).c_str());

	setTexVector(textures, tex_dir, tex_idx, objMaterials);
	resize_obj(TOshapes);

	if (!rc) {
		cerr << errStr << endl;
	}
	else {
		initTOShapes(rc, TOshapes);
	}
	
	num_mats = objMaterials.size();
}

