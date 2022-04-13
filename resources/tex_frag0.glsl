#version 330 core
uniform sampler2D Texture0;

in vec2 vTexCoord;
in vec3 emission;
in vec3 normal;
in vec3 lightDir;
in vec3 EPos;
in float shine;
out vec4 Outcolor;

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);
  vec3 n = normalize(normal);
  vec3 view = normalize(EPos);

  vec3 light = normalize(lightDir);
  vec3 h_angle = normalize(view+light);
  float dC = max(dot(normal,light), 0);
  float sC = max(dot(n, h_angle), 0);
  vec4 pointLightComp = texColor0*0.1+ texColor0*dC + texColor0*0.5*(pow(sC,shine));

  vec3 moonDir = normalize(vec3(-0.1, 1,-0.1)); //TODO: fix
  float moonStrength = 0.7;
  vec4 moonColor = vec4(50.0/255.0, 80.0/255.0, 210.0/255.0, 1.0f)*vec4(moonStrength,moonStrength, moonStrength, 1.0f);
  vec3 moon_h_angle = normalize(view+moonDir);
  float moon_dC = max(dot(normal,moonDir), 0);
  float moon_sC = max(dot(n, moon_h_angle), 0);
  vec4 moonLightComp = texColor0*0.05*moonColor + texColor0*moon_dC*moonColor + texColor0*0.5*(pow(moon_sC,shine))*moonColor;

  vec4 e = vec4(emission.xyz, 0.0);
  
  Outcolor = e + pointLightComp+ moonLightComp;
  //Outcolor = vec4(h_angle, 1);

  	//to set the outcolor as the texture coordinate (for debugging)
	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}