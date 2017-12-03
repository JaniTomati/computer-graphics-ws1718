#version 150

in vec2 texture_Coordinates;

uniform samplerCube cube_Map;

out vec4 out_Color;

void main() {
  out_Color = texture(cube_Map, texture_Coordinates);
}
