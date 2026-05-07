#version 330 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in float a_Intensity;

out float v_Intensity;

const int GRID_SIZE = 28;

void main() {
	int x = gl_InstanceID % GRID_SIZE;
	int y = -gl_InstanceID / GRID_SIZE;
	vec2 offset = vec2(x, y) * 2.0f / GRID_SIZE;
  gl_Position = vec4(a_Position + offset, 0.0f, 1.0f);
	v_Intensity = a_Intensity;
}
