#version 150

in vec3 eyeDir;

uniform samplerCube cube_Map;

out vec4 out_Color;

void main() {
  out_Color = texture(cube_Map, eyeDir);
}
