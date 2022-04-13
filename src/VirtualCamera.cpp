#include "VirtualCamera.h"
//#include "Spline.h"

using namespace glm;

VirtualCamera::VirtualCamera(vec3 playerStart, vec3 cameraStart) {
	pos = cameraStart + camera_offset;
	goCamera = false;
	lookAt = lookAt_default;
}

VirtualCamera::VirtualCamera() {}