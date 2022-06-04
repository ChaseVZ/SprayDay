#include "Tree.h"
#include <iostream>
#include <list>
#include "EcsCore/Coordinator.h"

using namespace std;
using namespace glm;

extern Coordinator gCoordinator;

bool Tree::isEntityInside(vec3 entPos, Tree::TreeNode node) {
	if (sqrt( (entPos.x-node.pos.x)*(entPos.x-node.pos.x) + (entPos.z-node.pos.z)*(entPos.z-node.pos.z) ) < node.radius) {
		return true;
	}
	return false;
}

Tree::TreeNode Tree::initTree(set<Entity> mEntities) {
	Tree::TreeNode root;
	root.pos = vec3(0, 0, 0);
	root.radius = 160.0f;
	bool entityAdded;
	//cout << "initTree: "<<endl;
	int count =0;
	for (Entity const& entity : mEntities) {
		//cout << "entity #" << count << endl;
		count++;
		entityAdded = false;
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		Tree::TreeNode n = root;
		while (!entityAdded) {
			if (Tree::isEntityInside(tr.pos, n)) {
				if (n.radius == 5) { //at the very bottom (leafnode) of tree, now need to add entity to tree
					//cout << "adding entity to tree" << endl;
					if (n.entities.empty()) {
						vector<Entity> entities;
						n.entities = entities;
						n.entities.push_back(entity);
					}
					else {
						n.entities.push_back(entity);
					}
					entityAdded = true;
				}
				else if (n.children.empty()) { //mid node, no children nodes=>  make new child node
					//cout << "mid node made and added"<<endl;
					vector<TreeNode> newChildren;
					n.children = newChildren;
					Tree::TreeNode child;
					child.pos = tr.pos;
					child.radius = n.radius/2;
					n.children.push_back(child);
					n = child;
				}
				else { //mid node, children exist
					//loop through children, check if in any, if not make new node and add to n.children
					//otherwise, if in child nodes, just continue?
					//cout << "adding another mid node" <<endl;
					bool foundTreeNode = false;
					for (Tree::TreeNode c : n.children) {
						if (Tree::isEntityInside(tr.pos, c)){
							n = c;
							foundTreeNode = true;
						}
					}
					if (!foundTreeNode) { //make new node and add to tree
						Tree::TreeNode child;
						child.pos = tr.pos;
						child.radius = n.radius/2;
						n.children.push_back(child);
						n = child;
					}
				}
			}
		}
	}
	return root;
		
}
