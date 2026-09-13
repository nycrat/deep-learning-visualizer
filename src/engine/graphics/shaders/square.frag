#version 330 core

in float v_Intensity;

out vec4 FragColor;

void main(){
  FragColor = vec4(vec3(1.0f, 1.0f, 1.0f) * v_Intensity, 1.0f);
}
