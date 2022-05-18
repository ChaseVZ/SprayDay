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
#include "../EcsCore/System.h"
#include "../Components/Transform.h"


class RenderSys : public System
{
public:
	void init(float grndSize);
	void update(shared_ptr<MatrixStack> Projection, mat4 View, GLuint depthMap, mat4 LSpace, bool isGrey);
	void drawDepth(shared_ptr<Program> curS);
private:
	Entity mCamera;
	GLuint mVao{};
	GLuint mVboVertices{};
	GLuint mVboNormals{};
	vec3 lightPos = vec3(10, 10, 10);
	void drawShadows(RenderComponent* rc, Transform* tr, shared_ptr<Program> curS);
	void draw(shared_ptr<MatrixStack> Projection, mat4 View,
	 RenderComponent* rc, Transform* tr, GLuint depthMap, mat4 LSpace, bool isGrey);
};

namespace RenderSystem {
	void drawGround(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View,
		 shared_ptr<Texture> grassTexture, bool isGrey);

}