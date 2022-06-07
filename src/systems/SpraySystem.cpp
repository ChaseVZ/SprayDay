#include "SpraySystem.h"
#include <iostream>


extern Coordinator gCoordinator;
particleGen* myPartGen;

float TIME_UNTIL_SPRAY = .15;
float timeSinceLastSpray = 0;

void SpraySys::generateSpray(vector<Entity>* trail, vec3 playerPos) {
	Entity sprayEnt = gCoordinator.CreateEntity();
	gCoordinator.AddComponent(
		sprayEnt,
		RenderComponent{
			sphere, //ShapeGroup*
			0.4,  //transparency
			texProg,
			GL_BACK,
		});
	gCoordinator.AddComponent(
		sprayEnt,
		Transform{
		playerPos,		//vec3 pos;
		vec3(1.0, 0.0, 0.0),     // vec3 rotation
		vec3(1.0),		//vec3 scale;
		});
	/*
	particleGen* sprayParticleGen = new particleGen(playerPos, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.1f, 0.4f); // start off screen
	sprayParticleGen->setnumP(90);
	sprayParticleGen->gpuSetup();
	gCoordinator.AddComponent(
		sprayEnt,
		ParticleComponent{
			sprayParticleGen
		});
		*/

	myPartGen->initParticleGroup(PARTICLES_PER_SPRAY, playerPos, sprayEnt);
	trail->push_back(sprayEnt);
}
void SpraySys::init(ShapeGroup* spherePtr, shared_ptr<Program> texProgPtr, particleGen* pg) {
	myPartGen = pg;
	sphere = spherePtr;
	texProg = texProgPtr;
}

void SpraySys::update(float frametime, vector<Entity> * trail, int mvmType, vec3 playerPos) {
	timeSinceLastSpray += frametime;
	for (int i = 0; i < trail->size(); i++) {
		RenderComponent* sprayRC = &(gCoordinator.GetComponent<RenderComponent>((*trail)[i]));
		Transform* sprayTR = &(gCoordinator.GetComponent<Transform>((*trail)[i]));
		sprayTR->scale += 0.15 * frametime;
		sprayRC->transparency -= 0.005 * frametime;
		if (sprayTR->scale.x >= 3) {
			//ParticleComponent* pc = &(gCoordinator.GetComponent<ParticleComponent>((*trail)[i]));
			//delete pc->partGen;
			
			myPartGen->deleteOldestParticleGroup(PARTICLES_PER_SPRAY, (*trail)[i]);
			gCoordinator.DestroyEntity((*trail)[i]);
			trail->erase(trail->begin() + i);			
			i -= 1;
		}
	}
	
	if (timeSinceLastSpray >= TIME_UNTIL_SPRAY) {
		if (mvmType == 1) {
			timeSinceLastSpray -= TIME_UNTIL_SPRAY;
			generateSpray(trail, playerPos);
		}
		else {
			timeSinceLastSpray = TIME_UNTIL_SPRAY; // cap time
		}
	}
}