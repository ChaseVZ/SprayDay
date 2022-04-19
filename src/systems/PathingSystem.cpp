#include "PathingSystem.h"
#include "../Enemy.h"
#include "../Player.h"
#include "../Program.h"
#include "../GameManager.h"
#include "../MatrixStack.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace PathingSystem {

    // Enemy Enemy(vec3 position, vec3 velocity, float boundingRadius) {
    //     pos = position;
    //     vel = velocity;
    //     boRad = boundingRadius;
    //     exploding = false;
    //     explodeFrame = 0;
    //     scale = 1.0;
    // };


    vec3 faceAway(vec3 p1, vec3 p2) {
		return normalize(vec3(p1.x - p2.x, 0.0, p1.z - p2.z)* length(p1))*vec3(0.2);
	}

    vec3 calcScareVel(vec3 ePos, vec3 pPos) {
		return normalize(vec3(ePos.x - pPos.x, 0.21, ePos.z - pPos.z));
	}


    bool checkCollisions(int sID, vector<Enemy>* enemies) {
		for (int i = 0; i < enemies->size(); i++) {
			if (i != sID && !(*enemies)[sID].exploding ) {
				if (length(vec3((*enemies)[sID].pos - (*enemies)[i].pos)) < (*enemies)[sID].boRad*2) {
					(*enemies)[sID].vel = faceAway((*enemies)[sID].pos, (*enemies)[i].pos);
					//enemies[sID].vel = vec3(0, 0.8, 0);
					return true;
				}
			}
		}
		return false;
	}

    bool collide(vec3 nextPos, Player p, Enemy* e) {
        if (nextPos.x + e->boRad > 125 || nextPos.x - e->boRad < -125)
        {
            e->vel = vec3(-1*(e->vel.x), e->vel.y, e->vel.z);
            return true;
        }
        if (nextPos.z + e->boRad> 125 || nextPos.z - e->boRad < -125)
        {
            e->vel = vec3(e->vel.x, e->vel.y, -1*(e->vel.z));
            return true;
        }

        if (sqrtf(pow((nextPos.x - p.pos.x), 2) + pow((nextPos.z - p.pos.z), 2)) < e->boRad + p.boRad) 
        {
            explode(e);
            return true;
        }
        return false;
    }


    void explode(Enemy* e) {
        e->exploding = true;
        GameManager* gm = gm->GetInstance();
        gm->setCollision(true);
    }

    void move(Player p, float dt, Enemy* e) {
        if (!collide(e->pos + e->vel*dt, p, e))
        {
            e->pos += e->vel*dt;
        }
    }

    void updateEnemies(shared_ptr<MatrixStack> Projection, mat4 View, float frametime, vector<Enemy>* enemies, Player player, shared_ptr<Program> texProg) {
		vector<int> toRemove;
		bool delta = false;

		for (int i = 0; i < enemies->size(); i++) {
			checkCollisions(i, enemies);
		}
		
		for (int i = 0; i < enemies->size(); i++) {
			
			move(player, frametime*50, &(*enemies)[i]);

			if ((*enemies)[i].exploding)
			{
				if ((*enemies)[i].explodeFrame == 0) {
					//numFlying += 1;
					(*enemies)[i].vel = calcScareVel((*enemies)[i].pos, player.pos);
					//cout << "caught one! " << enemies.size()-numFlying << " grounded skunks remaining" <<endl;
				}
				(*enemies)[i].explodeFrame += 1;
				if ((*enemies)[i].scale < 0.1) {
					toRemove.push_back(i); 
					//numFlying -= 1;
				}
				else { //drawSkunk(texProg, Projection, View, enemies[i], enemies[i]->scale - 0.0005); 
                    (*enemies)[i].scale -= 0.0005; 
                }
			}
			// else {
			// 	drawSkunk(texProg, Projection, View, enemies[i], 1);
			// }
		}

		for (int i : toRemove)
		{
			delta = true;
			enemies->erase(enemies->begin() + i);
		}

		//if (delta) { cout << enemies.size() << " skunks remaining!" << endl; }
	}
}

