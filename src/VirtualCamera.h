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
	VirtualCamera(vec3 playerStart, vec3 cameraStart);
	VirtualCamera();
	vec3 pos;
	vec3 lookAt;
	vec3 oldLookAt = lookAt_default;
	void updateUsingCameraPath(float frametime);
	void updatePos(vec3 playerPos) { 
		pos = playerPos + camera_offset; 

		// vec3 planeView; //vector from center of plane to direction of change, can be changed with input
		// planeView.x = cos(phi)*cos(theta);
		// planeView.y = sin(phi);
		// planeView.z = cos(phi)*cos(M_PI/2-theta);
		

		// vec3 transDir;
		
		// transDir.z = planeView.z/10*pVelocity;
		// transDir.x = planeView.x/10*pVelocity;
		// transDir.y = planeView.y/10*pVelocity;

		// vec3 oldSpitPos = gSpitPos;
		// gSpitPos.x += transDir.x;
		// gSpitPos.y += transDir.y;
		// if (gSpitPos.y < minPlaneY && pVelocity <= P_TAKEOFF_OR_LAND_SPEED) {
		// 	gSpitPos.y = minPlaneY;
		// }
		// else if (gSpitPos.y < minPlaneY) {
		// 	gSpitPos.y = minPlaneY-0.1;
		// }
		// gSpitPos.z += transDir.z;

		// vec3 cameraPosition = gSpitPos-(3.0f*planeView+abs(pVelocity)*planeView);
		// vec3 negCameraPos = gSpitPos+(3.0f*planeView+abs(pVelocity)*planeView);
		// vec3 cameraTarget = gSpitPos;
		// vec3 cameraUp = up;	

		// View->lookAt(cameraPosition, cameraTarget, up);
		//View->lookAt(playerPos-camera_offset, playerPos, vec3(0, 1, 0));
	};
	void resetCameraLookAt() { lookAt = lookAt_default; }
	bool goCamera;

private:
	Spline intro_path[11];
	Spline intro_path_lookAt[11];
	//vec3 camera_offset = vec3(0, 2, -2);
	vec3 camera_offset = vec3(0, 2, 0);
	vec3 lookAt_default = vec3(0, 0, -1);
};


#endif