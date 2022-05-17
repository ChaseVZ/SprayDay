#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform mat4 LS;
uniform vec3 lightPos;
uniform float alpha;
uniform bool useCubeTex;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;
out vec2 vTexCoord;
out vec3 texCoords;

out OUT_struct {
	vec3 fPos;
	vec3 fragNor;
	vec2 vTexCoord;
	vec4 fPosLS;
	vec3 vColor;
} out_struct;

float lightDist = 1.0;

void main() {

  /* First model transforms */
  vec3 wPos = vec3(M * vec4(vertPos.xyz, 1.0));
  gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

  fragNor = (V*M * vec4(vertNor, 0.0)).xyz;
  //lightDir = (V*(vec4(lightPos - wPos, 0.0))).xyz;
  lightDir = (V*vec4(1.0*lightDist, 2.0*lightDist, 2.0*lightDist, 0.0)).xyz;
  EPos = (V * vec4(wPos, 1.0)).xyz;

  /* the position in world coordinates */
  out_struct.fPos = (M*vec4(vertPos, 1.0)).xyz;
	/* the normal */
  out_struct.fragNor = (M*vec4(vertNor, 0.0)).xyz;
  /* pass through the texture coordinates to be interpolated */
  out_struct.vTexCoord = vertTex;
  /* The vertex in light space */
  out_struct.fPosLS = LS*vec4(out_struct.fPos, 1.0);
  /* a color that could be blended - or be shading */
  out_struct.vColor = vec3(max(dot(out_struct.fragNor, normalize(lightPos)), 0));
  
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;
  texCoords = vertPos;
}
