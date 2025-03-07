#version 330 core

in vec2 TexCoord;

out vec4 colour;

uniform sampler2D text;
uniform vec3 textColour;

void main(void) {
  float alpha = texture(text, TexCoord).r;
  vec4 sampled = vec4(1.0, 1.0, 1.0, alpha);

  colour = vec4(textColour, 1.0) * sampled;
}
