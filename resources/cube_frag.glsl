#version 330 core
uniform bool isGrey;
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main() {
  vec4 color;
  color = texture(skybox, TexCoords);
  //FragColor = vec4(TexCoords, 1.0);
  if(isGrey){
  FragColor = vec4(vec3((color.x+color.y+color.z)/3.0), color.w);
  } else{
  FragColor = texture(skybox, TexCoords);
  }
}
