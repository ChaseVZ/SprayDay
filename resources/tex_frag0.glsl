#version 330 core
uniform sampler2D Texture0;
uniform int flip;

in vec3 fragNor;
in vec3 lightDir;
in vec3 EPos;
in vec2 vTexCoord;
out vec4 Outcolor;

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);

	// My Code - Lab7
	vec3 matDif = texColor0.xyz;
	vec3 matAmb = matDif * 0.18;
	vec3 matSpec = matDif * 0.3;

    vec3 normal = normalize(fragNor) * flip;
	vec3 light = normalize(lightDir);
	vec3 cam = normalize(-1 * EPos);

	float Dc = max(0.0, dot(normal, light));
	vec3 h = (cam + light) / 2.0;
	float Sc = pow(max(0.0, dot(normal, h)), 1);

	vec4 color = vec4(matDif * Dc + matAmb + matSpec * Sc, 1.0);

	//if (color.g > 0.5)
	//	discard;

	//Outcolor = texColor0;
	Outcolor = color;
	//Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 1);
}
