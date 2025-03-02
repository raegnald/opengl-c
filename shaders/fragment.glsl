#version 330 core

in vec2 vert;
out vec4 FragColour;

void main(void) {
  FragColour = vec4(vert.x, vert.y, 1.0, 1.0);
}
