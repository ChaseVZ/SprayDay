#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

enum CollisionT : int {
	GROUND = 0,
	CRATE,
	RAMP,
	CUBE
};

struct CollisionComponent {
	vec3 center;
	int length; // in x direction
	int width;  // in z direction
	CollisionT c;
	float height = 4.0;
	vec3 dir = vec3(1, 0, 0);
	float slope = 0;
	float lowerBound = -1;
	float upperBound = -1;
};

struct CollisionOutput {
	float height;
	vec3 dir;
	bool isCollide;
	vec2 colPos;
	float slope;
	vec3 slopeDir;
};