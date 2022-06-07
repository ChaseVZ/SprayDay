#pragma once

#include "Shape.h"
#include "Texture.h"
#include "Program.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Components/Collision.h"

using namespace std;
using namespace glm;

struct ShapeGroup {
	vector<shared_ptr<Shape>> shapes;
	vector<shared_ptr<Texture>> textures;
	string filename;

	float startTime = 0.0f;
	float speed = 0.0f;
	Bone skeleton;
	Animation animation;
	uint boneCount;
	uint vao;
	std::vector<uint> indices;
	std::vector<glm::mat4> currentPose;
	glm::mat4 globalInverseTransform;
	float animDur = 0.0f;
};

enum ParticleT : int {
	NO_PARTICLES = 0,
	PURPLE_POOF
};

struct RenderComponent {
	ShapeGroup* sg;
	float transparency;
	shared_ptr<Program> shader;
	int cullDir;
	unsigned int texID = 999;
	bool isSkeletal = false;
	ShapeGroup* sg2;
	bool isSkunk = false;
	float skunkSpeed = 0.0f;
	float prevTailRot = -999;
};