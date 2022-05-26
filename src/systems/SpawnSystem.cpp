
#include "SpawnSystem.h"
#include <iostream>

extern Coordinator gCoordinator;
float WOLF_BASE_HP = 2.0; // seconds of spraying until death (if divisible by tick time)
float BEAR_BASE_HP = 4.0;
float STARTING_SPAWN_TIME = 6.0;
float TIME_TO_FIRST_SPAWN = 2.0;
float MIN_SPAWN_TIME = 0.5;
float SPAWN_TIME_DECREASE = .002; // every 1000 seconds, increase spawn time by 1 sec
float spawnTime;
float spawnTimer;


float randFloat() {
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return r;
}
vec3 SpawnSys::getRandStart() {
	return vec3((rand() % 2) * 2 - 1, 0, (rand() % 2) * 2 - 1) * float((MAP_SIZE /2.3));
}

void SpawnSys::initBear() {
	Entity bearEnt = gCoordinator.CreateEntity();
	vec3 startPos = getRandStart();
	gCoordinator.AddComponent(
		bearEnt,
		Transform{
			startPos,
			vec3(1.0, 0.0, 0.0), //lookdir
			vec3(5.0), //scale
			vec3(0.0,-3.14/2, 0.0) //rotation
		});

	gCoordinator.AddComponent(
		bearEnt,
		Enemy{
			2.0, // float boRad;
			startPos, // vec3 vel; (next move pos)
			startPos, //vec3 nextTile
			false, // bool exploding;
			0, // int explodeFrame;
			0.8
		});

	gCoordinator.AddComponent(
		bearEnt,
		DamageComponent{
			BEAR_BASE_HP + POISON_TICK_TIME, // total hp, tick time is added because of tick calculations
			BEAR_BASE_HP + POISON_TICK_TIME, // current hp
			0.0 // poison timer
		});

	gCoordinator.AddComponent(
		bearEnt,
		AnimationComponent{
			false,
			0 // poision damage frame
		});

	gCoordinator.AddComponent(
		bearEnt,
		RenderComponent{
			bear,
			1.0,
			texProg,
			GL_BACK,
		});
}

void SpawnSys::initWolf() {
	Entity wolfEnt = gCoordinator.CreateEntity();
	vec3 startPos = getRandStart();
	gCoordinator.AddComponent(
		wolfEnt,
		Transform{
			startPos,
			vec3(1.0, 0.0, 0.0),
			vec3(5.0),
		});

	gCoordinator.AddComponent(
		wolfEnt,
		Enemy{
			2.0, // float boRad;
			startPos, // vec3 vel;
			startPos, //start poosition (nextTile)
			false, // bool exploding;
			0, // int explodeFrame;
			1.0 //SPEED
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
			999,
			true // isSkeletal
		});

	gCoordinator.AddComponent(
		wolfEnt,
		SkeletalComponent{
			wolf->filename.c_str()
		});
}

void SpawnSys::spawnEnemy() {
	spawnTimer -= spawnTime;
	int randEnemySpawn = rand() % 2;
	if (randEnemySpawn == 1) {
		initBear();
	}
	else {
		initWolf();
	}
}
void SpawnSys::init(int mapSize, float poisonTickTime, ShapeGroup* wolfPtr, ShapeGroup* bearPtr, shared_ptr<Program> texProgPtr) {
	reset();
	MAP_SIZE = mapSize;
	wolf = wolfPtr;
	bear = bearPtr;
	texProg = texProgPtr;
	POISON_TICK_TIME = poisonTickTime;
}
void SpawnSys::update(float frameTime) {
	spawnTime = (std::max)(spawnTime - SPAWN_TIME_DECREASE * frameTime, MIN_SPAWN_TIME);
	spawnTimer += frameTime;
	if (spawnTimer > spawnTime) {
		spawnEnemy();
	}
}

void SpawnSys::reset() {
	spawnTime = STARTING_SPAWN_TIME;
	spawnTimer = STARTING_SPAWN_TIME - TIME_TO_FIRST_SPAWN;
}