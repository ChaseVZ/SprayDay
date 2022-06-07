#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 boneIds;
layout(location = 5) in vec4 boneWeights;

uniform mat4 bone_transforms[50];
uniform bool isSkeletal;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform mat4 LS;
uniform vec3 lightPos;
uniform float alpha;

out vec4 bw;
out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;
out vec2 vTexCoord;
out vec3 texCoords;
out vec4 fPosLS;

float lightDist = 1.0;

void main() {

	mat4 boneTransform  =  mat4(0.0);
	vec3 fPos;


	if (isSkeletal){
		bw = vec4(0);

		//if(int(boneIds.x) == 1)
		//	bw.z = boneIds.x;
		//boneWeights = normalize(boneWeights);

		boneTransform  +=    bone_transforms[int(boneIds.x)] * boneWeights.x;
		boneTransform  +=    bone_transforms[int(boneIds.y)] * boneWeights.y;
		boneTransform  +=    bone_transforms[int(boneIds.z)] * boneWeights.z;
		boneTransform  +=    bone_transforms[int(boneIds.w)] * boneWeights.w;
	}

	/* First model transforms */
	vec3 wPos;

	if (isSkeletal) {
		wPos = vec3(M * vec4(vertPos.xyz, 1.0) * boneTransform);
		gl_Position = P * V * M * boneTransform * vec4(vertPos.xyz, 1.0);

		fragNor = mat3(transpose(inverse(M * boneTransform))) * vertNor; // SKELETAL ("v_normal" = "fragNor" i think; "normal" = "verNor")
		fragNor = normalize(fragNor); // SKELETAL

		fPos = (M*vec4(vertPos, 1.0)).xyz;
	}
	else{
		wPos = vec3(M * vec4(vertPos.xyz, 1.0));
		gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

		fragNor = (V * M * vec4(vertNor, 0.0)).xyz;

		fPos = (M*vec4(vertPos, 1.0)).xyz;
	}		

	lightDir = (V*vec4(1.0*lightDist, 2.0*lightDist, 2.0*lightDist, 0.0)).xyz;
	
	EPos = (V * vec4(wPos, 1.0)).xyz;

	//fPos = (M*vec4(vertPos, 1.0)).xyz;

	fPosLS = LS*vec4(fPos, 1.0);
  
	/* pass through the texture coordinates to be interpolated */
	if (isSkeletal){
		vTexCoord = uv;
		//vTexCoord = vertTex;
		//vTexCoord = vec2(0,0);
	}
	else
		vTexCoord = vertTex;
	
	//vTexCoord = vertTex; // uv is not working
	texCoords = vertPos; // for cubes
}
