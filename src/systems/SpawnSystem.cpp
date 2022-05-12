#include "SpawnSystem.h"

extern Coordinator gCoordinator;
float WOLF_BASE_HP = 4.0; // seconds of spraying until death (if divisible by tick time)
float spawnTimer = 3;
float SPAWN_TIME = 4;

float randFloat() {
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return r;
}
vec3 SpawnSys::getRandStart() {
	return vec3((rand() % 2) * 2 - 1, 0, (rand() % 2) * 2 - 1) * float((MAP_SIZE / 3.0));
}

void SpawnSys::initWolf() {
	Entity wolfEnt = gCoordinator.CreateEntity();
	gCoordinator.AddComponent(
		wolfEnt,
		Transform{
			getRandStart(),
			vec3(1.0, 0.0, 0.0),
			vec3(5.0),
		});

	gCoordinator.AddComponent(
		wolfEnt,
		Enemy{
			2.0, // float boRad;
			vec3(randFloat() / 4.0 - 0.125, 0, randFloat() / 4.0 - 0.125), // vec3 vel;
			false, // bool exploding;
			0, // int explodeFrame;
		});

	gCoordinator.AddComponent(
		wolfEnt,
		DamageComponent{
			WOLF_BASE_HP + POISON_TICK_TIME, // total hp, tick time is added because of tick calculations
			WOLF_BASE_HP + POISON_TICK_TIME, // current hp
			0.0 // poison timer
		});

	gCoordinator.AddComponent(
		wolfEnt,
		AnimationComponent{
			false,
			0 // poision damage frame
		});

	gCoordinator.AddComponent(
		wolfEnt,
		RenderComponent{
			wolf,
			1.0,
			texProg,
			GL_BACK,
		});
}

void SpawnSys::spawnEnemies(float frametime) {
	spawnTimer += frametime;
	if (spawnTimer > SPAWN_TIME) {
		spawnTimer -= SPAWN_TIME;
		initWolf();
	}
}
void SpawnSys::init(int mapSize, float poisonTickTime, ShapeGroup* wolfPtr, shared_ptr<Program> texProgPtr){
	MAP_SIZE = mapSize;
	wolf = wolfPtr;
	texProg = texProgPtr;
	POISON_TICK_TIME = poisonTickTime;
}
void SpawnSys::update(float frameTime){
	spawnEnemies(frameTime);
}