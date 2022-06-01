#ifndef ASTAR_H
#define ASTAR_H


#include "./Components/Transform.h"
#include "Player.h"
#include "GameManager.h"
#ifndef COLL_SYS
    #define COLL_SYS
    #include "systems/CollisionSystem.h"
#endif
#include <vector>

enum PathingT : int {
	SIMPLE_PATH = 0,
	FLANK_PATH,
};

class Astar
{
    public: 
        static vec3 findNextPos(Player p, Transform* tr, shared_ptr<CollisionSys> collSys, PathingT);

};

#endif