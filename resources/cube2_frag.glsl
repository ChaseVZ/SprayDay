#version 330 core
uniform samplerCube cubeTex;
uniform int flip;
uniform float alpha;
uniform bool useCubeTex;

in vec3 texCoords;
in vec3 fragNor;
in vec3 lightDir;
in vec3 EPos;
in vec2 vTexCoord;
out vec4 Outcolor;

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

	vec4 color = vec4(matDif * Dc + matAmb + matSpec * Sc, alpha);
	//vec4 color = vec4(matSpec*Sc, 1.0);

	//if (color.g > 0.5)
	//	discard;

	//Outcolor = texColor0;
	Outcolor = color;
	//Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 1);
}
