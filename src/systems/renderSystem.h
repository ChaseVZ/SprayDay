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
#include "../Components/AnimationComponent.h"
#include "../Tree.h"
#include "../Text.h"
#include <vector>


class RenderSys : public System
{
public:
	void init(float grndSize, shared_ptr<Program> ptProg, shared_ptr<Texture> ptTex);
	void update(shared_ptr<MatrixStack> Projection, mat4 View, GLuint depthMap, mat4 LSpace, bool isGrey, float gameTime);
	void drawDepth(shared_ptr<Program> curS);
	int ViewFrustCull(vec3 center, float radius);
	float DistToPlane(float A, float B, float C, float D, vec3 point);
	void ExtractVFPlanes(mat4 P, mat4 V);
	Tree::TreeNode getTree();
private:
	Entity mCamera;
	GLuint mVao{};
	GLuint mVboVertices{};
	GLuint mVboNormals{};
	vec3 lightPos = vec3(5, 3, 5);
	void drawShadows(RenderComponent* rc, Transform* tr, shared_ptr<Program> curS);
	void draw(shared_ptr<MatrixStack> Projection, mat4 View, RenderComponent* rc, Transform* tr, GLuint depthMap, mat4 LSpace, bool isGrey, float gameTime);
	void drawSkeletal(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, shared_ptr<Texture> tex, float elapsedTime, SkeletalComponent sc, 
		shared_ptr<Program> curS, GLuint depthMap, mat4 LSpace, RenderComponent* rc, Transform* tr);
	void drawSprayParticles(mat4 view, mat4 projection, mat4 model);
};

namespace RenderSystem {
	void drawGround(shared_ptr<Program> curS, shared_ptr<MatrixStack> Projection, mat4 View,
		 shared_ptr<Texture> grassTexture, bool isGrey, GLuint depthMap);
	void drawText(shared_ptr<Program> textProg, bool gameOver, unsigned int TextVAO, unsigned int TextVBO, std::map<GLchar, MyText::Character> Characters, int gameTime, int enemiesKilled);

}