	
#include "SpawnSystem.h"
#include <iostream>

extern Coordinator gCoordinator;
float WOLF_BASE_HP = 3.0; // seconds of spraying until death (if divisible by tick time)
float BEAR_BASE_HP = 12.0;
float STARTING_SPAWN_TIME = 6.0;
float TIME_TO_FIRST_SPAWN =  2.0;
float MIN_SPAWN_TIME = 0.5;
float SPAWN_TIME_DECREASE = .005; // every 1/spawn_time_decrease seconds, increase spawn time by 1 sec
float spawnTime;
float spawnTimer;


float randFloat() {
	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	return r;
}
vec3 SpawnSys::getRandStart() {
	return vec3((rand() % 2) * 2 - 1, 0.0, (rand() % 2) * 2 - 1) * float((MAP_SIZE /2.3));
}

void SpawnSys::initBear(float gameTime) {
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
			0.6, // speed
			SIMPLE_PATH
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
			999,
			true // isSkeletal
		});
	//gCoordinator.AddComponent(
	//	bearEnt,
	//	SkeletalComponent{
	//		bear->filename,
	//		gameTime,
	//		35.0f
	//	});
	gCoordinator.AddComponent(
		bearEnt,
		SkeletalComponent{
			bear->filename.c_str(),
			gameTime,
			35.0f,
			bear->skeleton,
			bear->animation,
			bear->boneCount,
			bear->vao,
			bear->indices,
			bear->currentPose,
			bear->globalInverseTransform,
			bear->animDur
		});
}

void SpawnSys::initWolf(float gameTime) {
	Entity wolfEnt = gCoordinator.CreateEntity();
	vec3 startPos = getRandStart();
	gCoordinator.AddComponent(
		wolfEnt,
		Transform{
			startPos,
			vec3(1.0, 0.0, 0.0),
			vec3(5.0),
			vec3(0.0,-3.14 / 2, 0.0) //rotation
		});

	gCoordinator.AddComponent(
		wolfEnt,
		Enemy{
			2.0, // float boRad;
			startPos, // vec3 vel;
			startPos, //start poosition (nextTile)
			false, // bool exploding;
			0, // int explodeFrame;
			1.1, //SPEED
			FLANK_PATH
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
	//gCoordinator.AddComponent(
	//	wolfEnt,
	//	SkeletalComponent{
	//		wolf->filename,
	//		gameTime,
	//		50.0f
	//	});
	gCoordinator.AddComponent(
		wolfEnt,
		SkeletalComponent{
			wolf->filename.c_str(),
			gameTime,
			50.0f,
			wolf->skeleton,
			wolf->animation,
			wolf->boneCount,
			wolf->vao,
			wolf->indices,
			wolf->currentPose,
			wolf->globalInverseTransform,
			wolf->animDur
		});
}

void SpawnSys::spawnEnemy(std::shared_ptr<AnimationSys> animationSys, float gameTime) {
	spawnTimer -= spawnTime;
	int randEnemySpawn = rand() % 2;
	if (randEnemySpawn == 1) {
		initBear(gameTime);
		//animationSys->init();
	}
	else {
		initWolf(gameTime);
		//animationSys->init();
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

void SpawnSys::update(float frameTime, std::shared_ptr<AnimationSys> animationSys, float gameTime) {
	spawnTime = (std::max)(spawnTime - SPAWN_TIME_DECREASE * frameTime, MIN_SPAWN_TIME);
	spawnTimer += frameTime;
	if (spawnTimer > spawnTime) {
		spawnEnemy(animationSys, gameTime);
	}
}

void SpawnSys::reset() {
	spawnTime = STARTING_SPAWN_TIME;
	spawnTimer = STARTING_SPAWN_TIME - TIME_TO_FIRST_SPAWN;
}