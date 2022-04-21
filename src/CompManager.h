#ifndef COMP_MANAGER_H
#define COMP_MANAGER_H

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include "DamageComponent.h"

using namespace glm;
using namespace std;
class CompManager
{
private:
	// instance
	static CompManager* instance;

	// private constructor
	CompManager() {};
public:

	// data
	vector<DamageComponent> damageComps;

	static CompManager* GetInstance()
	{
		if (!instance) { instance = new CompManager(); }
		return instance;
	}
};
#endif