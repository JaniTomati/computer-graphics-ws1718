#version 150

in vec3 view_Direction;

uniform samplerCube skybox;

out vec4 out_Color;

void main() {
  out_Color = texture(skybox, view_Direction);
}
