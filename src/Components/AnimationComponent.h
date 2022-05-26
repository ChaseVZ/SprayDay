#pragma once
#include "../animationUtil.h"

struct AnimationComponent {
	bool inPoison;
	int poisonDamageFrame;
};

struct SkeletalComponent {
	const char* filename;
	Bone skeleton;
	Animation animation;
	uint boneCount;
	uint vao;
	std::vector<uint> indices;
	std::vector<glm::mat4> currentPose;
	glm::mat4 globalInverseTransform;
};