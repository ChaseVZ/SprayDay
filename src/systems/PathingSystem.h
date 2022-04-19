#include "../Enemy.h"
#include "../Player.h"
#include "../Program.h"
#include "../MatrixStack.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

namespace PathingSystem {
	//Enemy Enemy(vec3 position, vec3 velocity, float boundingRadius);
    void move(Player p, float dt, Enemy* e);
    bool collide(vec3 nextPos, Player p, Enemy* e);
    void explode(Enemy* e);
    void updateEnemies(shared_ptr<MatrixStack> Projection, mat4 View, float frametime, vector<Enemy>* enemies, Player player, shared_ptr<Program> texProg);
    bool checkCollisions(int sID, vector<Enemy>* enemies);
    vec3 faceAway(vec3 p1, vec3 p2);
    vec3 calcScareVel(vec3 ePos, vec3 pPos);
}