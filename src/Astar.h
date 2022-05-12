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

class Astar
{
    public: 
        static vec3 findNextPos(Player p, Transform* tr, shared_ptr<CollisionSys> collSys);

};

#endif