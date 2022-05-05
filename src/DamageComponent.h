#ifndef DAMAGE_COMP
#define DAMAGE_COMP

#include <glm/gtc/type_ptr.hpp>

using namespace glm;

struct DamageComponent {
	float maxHp;
	float currentHp;
	float poisonTimer;
};

#endif