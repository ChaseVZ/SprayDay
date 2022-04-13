#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform int flip;
uniform vec3 lightPos;
uniform vec3 vEmission;
uniform float MatShine;
uniform float texMult;


out vec3 normal;
out vec3 lightDir;
out vec3 EPos;
out vec2 vTexCoord;
out float shine;
out vec3 emission;


void main() {

  /* First model transforms */
  vec3 wPos = vec3(M * vec4(vertPos.xyz, 1.0));
  gl_Position = P * V *M * vec4(vertPos.xyz, 1.0);

  shine = MatShine;
  normal = normalize ((V*M * vec4(vertNor, 0.0)).xyz*flip);
  lightDir = (V*(vec4(lightPos - wPos, 0.0))).xyz;
  vec3 negView = (V * vec4(wPos, 1.0)).xyz;
  EPos = vec3(-1)*negView;

  emission = vEmission;
  
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex*texMult;
}
