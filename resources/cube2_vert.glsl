#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lightPos;
uniform float alpha;
uniform bool useCubeTex;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;
out vec2 vTexCoord;
out vec3 texCoords;

float lightDist = 1.0;

void main() {

  /* First model transforms */
  vec3 wPos = vec3(M * vec4(vertPos.xyz, 1.0));
  gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

  fragNor = (V*M * vec4(vertNor, 0.0)).xyz;
  //lightDir = (V*(vec4(lightPos - wPos, 0.0))).xyz;
  lightDir = (V*vec4(1.0*lightDist, 2.0*lightDist, 2.0*lightDist, 0.0)).xyz;
  EPos = (V * vec4(wPos, 1.0)).xyz;
  
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
  texCoords = vertPos;
}
