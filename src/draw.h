#pragma once
#ifndef DRAW_INCLUDED
#define DRAW_INCLUDED

using namespace std;
using namespace glm;


void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS);
void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M);
void drawRocks(shared_ptr<Shape> rock, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg);
void drawGround(shared_ptr<MatrixStack> Model, shared_ptr<Program> curS, shared_ptr<Texture> texture2,
	GLuint GroundVertexArrayID, GLuint GrndBuffObj, GLuint GrndNorBuffObj, GLuint GrndTexBuffObj, GLuint GIndxBuffObj, int g_GiboLen);
void drawBoundary(vector<shared_ptr<Shape>> rock, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg);
void drawTree(vector<shared_ptr<Shape>> tree, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg, shared_ptr<Texture> leaf_tex);
#endif // DRAW_INCLUDED