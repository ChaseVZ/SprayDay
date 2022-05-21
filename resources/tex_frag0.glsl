#version 330 core
uniform samplerCube cubeTex;
uniform sampler2D shadowDepth;
uniform sampler2D Texture0;
uniform int flip;
uniform float alpha;
uniform bool useCubeTex;
uniform bool isGrey;

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
  float curD = projCoord.z - 0.005;
	// //2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  float lightDepth = texture(shadowDepth, projCoord.xy).r;
	// //3: compare to the current depth (.z) of the projected depth

	// //4: return 1 if the point is shadowed

  vec2 texelScale = 1.0 / textureSize(shadowDepth, 0);  
  //index into the texture using this scale to offset by 1-2 fragments: 
  float percentShadow = 0.0;
  for (int i=-2; i <= 2; i++) { 
  	for (int j=-2; j <= 2; j++) { 
    	lightDepth = texture(shadowDepth, projCoord.xy+vec2(i, j)*texelScale).r; 
     	if (curD > lightDepth) 
       		percentShadow += 0.5; 
   } 
  } 
  return percentShadow/25.0;

}

void main() {
	vec4 texColor0;
	/*
	if (useCubeTex){
		texColor0 = texture(cubeTex, texCoords);
	}
	else{
		texColor0 = texture(Texture0, vTexCoord);
	}
	*/
	texColor0 = texture(Texture0, vTexCoord);

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
	
	color = vec4((1.0-Shade)*color.xyz, color.w);
	if(isGrey){
	float averageCol = (color.x + color.y + color.z)/3.0;
	Outcolor = vec4(vec3(averageCol), color.w);
	}
	else{
	Outcolor = color;
	}

}
