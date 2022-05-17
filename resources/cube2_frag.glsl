#version 330 core
uniform samplerCube cubeTex;
uniform sampler2D shadowDepth;
uniform int flip;
uniform float alpha;
uniform bool useCubeTex;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;

in vec3 texCoords;
in vec3 fragNor;
in vec3 lightDir;
in vec3 EPos;
in vec2 vTexCoord;
out vec4 Outcolor;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float TestShadow(vec4 LSfPos) {

	// //1: shift the coordinates from -1, 1 to 0 ,1
  vec3 projCoord = 0.5*(LSfPos.xyz + vec3(1.0));
  float curD = projCoord.z - 0.001;
	// //2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  float lightDepth = texture(shadowDepth, projCoord.xy).r;
	// //3: compare to the current depth (.z) of the projected depth

	// //4: return 1 if the point is shadowed
  if (curD > lightDepth) {
    return 0.5f;
  }
  return 0.0f;
}

void main() {
	vec4 texColor0;
	if (useCubeTex){
		texColor0 = texture(cubeTex, texCoords);
	}

	vec3 matDif = texColor0.xyz;
	vec3 matAmb = matDif * 0.4;
	vec3 matSpec = matDif * 0.3;

    vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
	vec3 cam = normalize(-1 * EPos);

	float Dc = max(0.0, dot(normal, light));
	vec3 h = (cam + light) / 2.0;
	float Sc = pow(max(0.0, dot(normal, h)), 1);

	float Shade = TestShadow(in_struct.fPosLS);

	vec4 color = vec4(matDif * Dc + matAmb + matSpec * Sc, alpha);
	//vec4 color = vec4(matSpec*Sc, 1.0);

	//if (color.g > 0.5)
	//	discard;

	//Outcolor = texColor0;
	//Outcolor = amb*(texColor0) + (1.0-Shade)*texColor0*color;
	Outcolor = vec4((1.0-Shade)*color.xyz, color.w);
	//Outcolor = color;

	//Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 1);
}
