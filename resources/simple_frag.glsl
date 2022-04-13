#version 330 core 

out vec4 color;

// Ka
uniform vec3 MatAmb;
// Kd
uniform vec3 MatDif;
// Ks
uniform vec3 MatSpec;
// Alpha
uniform float MatShine;
uniform float alpha;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
	vec3 cam = normalize(EPos);

	// used with Kd
	float Dc = max(0.0, dot(normal, light));

	// used with Ks
	vec3 h = (cam + light) / 2.0;
	float Sc = pow(max(0.0, dot(normal, h)), MatShine);
	color = vec4(MatDif * Dc + MatAmb + MatSpec * Sc, 1.0);
	//color = vec4(vec3(MatSpec * Sc), );
}
