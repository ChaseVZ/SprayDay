#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

struct Transform {
	vec3 pos;
	vec3 lookDir;
	vec3 scale;
	vec3 rotation = vec3(0,0,0);
	vec3 rampRotation = vec3(0);
};