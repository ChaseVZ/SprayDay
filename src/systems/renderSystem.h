#pragma once

#include "../ShapeGroup.h"
#include "../GLSL.h"

#include "../Program.h"
#include "../MatrixStack.h"
#include "../particleSys.h"
#include "../GLSL.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace RenderSystem {
	mat4 lookDirToMat(vec3 lookDir);
	void draw(shared_ptr<MatrixStack> Projection, mat4 View, RenderComponent* rc);
	void draw(ShapeGroup sg, shared_ptr<Program> curS);
	void draw(ShapeGroup sg, shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View, vec3 trans, vec3 sc, vec3 rot, bool useLookAt, vec3 dir);
	void drawParticles(shared_ptr<Program> curS, shared_ptr<MatrixStack> P, mat4 View, vec3 pos, particleSys* partSys, shared_ptr<Texture> tex);
	void SetMaterial(shared_ptr<Program> curS, int i);

	void drawGround(shared_ptr<MatrixStack> Model, shared_ptr<Program> curS, shared_ptr<Texture> tex,
		GLuint GroundVertexArrayID, GLuint GrndBuffObj, GLuint GrndNorBuffObj, GLuint GrndTexBuffObj, GLuint GIndxBuffObj, int g_GiboLen);
	void drawObstacles(ShapeGroup sg, shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View);
}