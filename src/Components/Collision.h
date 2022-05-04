#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

enum CollisionT : int {
	CRATE = 1,
	RAMP,
	CUBE
};

struct CollisionComponent {
	int length; // in x direction
	int width;  // in z direction
	CollisionT c;
};