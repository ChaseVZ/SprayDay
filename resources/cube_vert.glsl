#version 330 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNor;
layout (location = 2) in vec2 vertTex; // unused on purpose
out vec3 TexCoords;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main() {
	TexCoords = vertPos;
	gl_Position = P*V*M*vec4(vertPos.xyz, 1.0);
}
