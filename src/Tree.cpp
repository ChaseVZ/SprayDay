#include "Tree.h"
#include <iostream>
#include <list>
#include "EcsCore/Coordinator.h"

using namespace std;
using namespace glm;

extern Coordinator gCoordinator;

bool debug = true;
int count1 = 0;

bool Tree::isEntityInside(vec3 entPos, Tree::TreeNode node) {
	if (sqrt( (entPos.x-node.pos.x)*(entPos.x-node.pos.x) + (entPos.z-node.pos.z)*(entPos.z-node.pos.z) ) < node.radius) {
		return true;
	}
	return false;
}

void Tree::TraverseTree(TreeNode tn) {
	// if (tn == null) {
	// 	return;
	// }
	cout << "Node " << count1 << " has " << tn.children.size() << " children\n";
	count1++;
	for (TreeNode c : tn.children) {
		Tree::TraverseTree(c);
	}
	cout<< "traversing" << endl;
	cout<<"TN: " << tn.pos.x << " " << tn.pos.z << " rad: " << tn.radius <<endl;
}

Tree::TreeNode Tree::initTree(set<Entity> mEntities) {
	int treenodeCount = 0;
	Tree::TreeNode root;
	root.pos = vec3(0, 0, 0);
	root.radius = 160.0f;
	root.nodeNum = treenodeCount;
	treenodeCount++;
	bool entityAdded;
	if (debug) { cout << "initTree: "<<endl; }
	int count =0;
	for (Entity const& entity : mEntities) {
		///if (count <8 ) {
		if (debug) { cout << "\nentity #" << count << endl; }
		count++;
		entityAdded = false;
		Transform& tr = gCoordinator.GetComponent<Transform>(entity);
		Tree::TreeNode* n = &root;
		while (!entityAdded) {
			if (debug) { cout << "Checking parent " << n->nodeNum << " with values:" <<endl;
						 cout << "   radius: " << n->radius << endl;
						 cout << "   pos: " << n->pos.x << " " << n->pos.z << endl;
						 cout << "   numChildren: " << n->children.size() << endl;}
			//if (Tree::isEntityInside(tr.pos, *n)) {
				if (n->radius == 10) { //at the very bottom (leafnode) of tree, now need to add entity to tree
					if (debug) { cout << "  adding entity to tree" << endl; }
					n->entities.push_back(entity);
					entityAdded = true;
				}
				else if (n->children.empty()) { //mid node, no children nodes=>  make new child node
					if (debug) { cout << "  parent " << n->nodeNum<< " no children, making new child"<<endl; }
					Tree::TreeNode child;
					child.pos = tr.pos;
					child.radius = n->radius/2;
					child.nodeNum = treenodeCount;
					n->children.push_back(child);
					n = &child;
					treenodeCount++;
				}
				else { //mid node, children exist
						//loop through children, check if in any, if not make new node and add to n.children
						//otherwise, if in child nodes, just continue?
					if (debug) { cout << "  parent " << n->nodeNum<< " children exist, checking for fit" <<endl; }
					bool foundTreeNode = false;
					for (Tree::TreeNode c : n->children) {
						if (debug) { cout << "    in " << n->nodeNum<< "->children, checking if entity in child" << endl; }
						if (Tree::isEntityInside(tr.pos, c)){
							n = &c;
							foundTreeNode = true;
							if (debug) { cout << "     found tree node parent " << c.nodeNum << "should be same as " << n->nodeNum << endl; }
							if (debug) { cout << "     found parent " << n->nodeNum << " with values:" <<endl;
											cout << "     radius: " << n->radius << endl;
											cout << "     pos: " << n->pos.x << " " << n->pos.z << endl;
											cout << "     numChildren: " << n->children.size() << endl; }
							break;
						}
					}
					if (!foundTreeNode) { //make new node and add to tree
						if (debug) { cout << "  didn't find fit, adding new node" << endl; }
						Tree::TreeNode child;
						child.pos = tr.pos;
						child.radius = n->radius/2;
						child.nodeNum = treenodeCount;
						n->children.push_back(child);
						n = &child;
						treenodeCount++;
					}
				}
				if (debug) {cout << "end loop"<<endl;
				            cout << "   Treenode made?: " << n->nodeNum-1 << " with values:" <<endl;
							cout << "        radius: " << n->radius << endl;
							cout << "        pos: " << n->pos.x << " " << n->pos.z << endl;
							cout << "        numChildren: " << n->children.size() << endl; }
			//}
		}
		///}
	}
	if (debug) { cout<< "num tree nodes " << treenodeCount << endl;}
	return root;
		
}
