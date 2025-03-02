#version 330 core

layout (location = 0) in vec2 pos;
out vec2 vert;

void main(void) {
  vert = pos;
  gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}
