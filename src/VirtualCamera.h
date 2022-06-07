#pragma once
#ifndef CHASE_CAMERA
#define CHASE_CAMERA

#include <iostream>
#include <glad/glad.h>
#include <vector>

#include "Spline.h"
#include <chrono> 
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

class VirtualCamera {
public:
	VirtualCamera(vec3 playerStart);
	VirtualCamera();
	vec3 pos;
	vec3 lookAt;
	vec3 oldLookAt = lookAt_default;
	void updateUsingCameraPath(float frametime);
	void updatePos(vec3 playerPos) { 
		pos = playerPos + camera_offset; 
	};
	void resetCameraLookAt() { lookAt = lookAt_default; }
	bool goCamera;

private:
	Spline intro_path[11];
	Spline intro_path_lookAt[11];
	//vec3 camera_offset = vec3(0, 2, -2);
	vec3 camera_offset = vec3(0, 0, 0);
	vec3 lookAt_default = vec3(0, 0, -1);
};


#endif