#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

struct Transform {
	vec3 pos;
	vec3 lookDir;
	vec3 scale;
};