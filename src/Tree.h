#pragma once

#include <string>
#include <vector>
#include <set>
#include "EcsCore/EcsTypes.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "Components/Transform.h"

using namespace std;
using namespace glm;

class Tree
{

public:
	typedef struct TreeNode {
		vec3 pos;
		float radius;
		vector<TreeNode> children;
		vector<Entity> entities;
		int nodeNum;
	} TreeNode;

	static TreeNode initTree(set<Entity> mEntities);
	static void TraverseTree(TreeNode tn);

private:
	static bool isEntityInside(vec3 entPos, TreeNode node);
};

