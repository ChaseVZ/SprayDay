#pragma once
#ifndef DRAW_INCLUDED
#define DRAW_INCLUDED

using namespace std;
using namespace glm;

void drawBridge( vector<shared_ptr<Shape>> bridge, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg, vec3 worldPivot, mat4 worldMat);
void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS);
void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M);
void drawRocks(shared_ptr<Shape> rock, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg);
void drawRobot(shared_ptr<Shape> sphere, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg, vec3 worldPivot,
	shared_ptr<Texture> head_tex, shared_ptr<Texture> eye_tex, float sTheta, mat4 spinMat, mat4 headLookMat);
vec3 getLampPos(shared_ptr<MatrixStack> Model, vec3 worldPivot, mat4 headLookMat);
void drawGround(shared_ptr<MatrixStack> Model, shared_ptr<Program> curS, vec3 worldPivot, mat4 worldMat, shared_ptr<Texture> texture2,
	GLuint GroundVertexArrayID, GLuint GrndBuffObj, GLuint GrndNorBuffObj, GLuint GrndTexBuffObj, GLuint GIndxBuffObj, int g_GiboLen);
void drawBoundary(vector<shared_ptr<Shape>> rock, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg);
void drawTree(vector<shared_ptr<Shape>> tree, shared_ptr<MatrixStack> Model, shared_ptr<Program> texProg, shared_ptr<Texture> leaf_tex);
#endif // DRAW_INCLUDED