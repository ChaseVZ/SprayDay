#pragma once
#include "EcsTypes.h"
#include <set>
class System
{
public:
	std::set<Entity> mEntities;
};