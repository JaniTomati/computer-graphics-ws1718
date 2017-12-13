#version 150

in vec2 texture_Coordinates;

uniform sampler2D FramebufferTex;

out vec4 out_Color;

void main() {
  out_Color = texture(FramebufferTex, texture_Coordinates);
}
