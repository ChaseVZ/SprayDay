#version 330 core
uniform bool isGrey;
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube cubeTex;

void main() {
  vec4 color;
  color = texture(cubeTex, TexCoords);
  //FragColor = vec4(TexCoords, 1.0);
  if(isGrey){
  FragColor = vec4(vec3((color.x+color.y+color.z)/3.0), color.w);
  } else{
  FragColor = texture(cubeTex, TexCoords);
  }
  if (color.x >= 0.99 && color.y >= 0.99 && color.z >= 0.99){
    FragColor.w = 0.0;
  }
}
