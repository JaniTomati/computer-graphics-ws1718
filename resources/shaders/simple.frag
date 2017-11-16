#version 150

in vec3 pass_Normal;
in vec3 camera_Position;
in vec3 planet_Color;
in vec3 world_Position;
in vec3 world_Normal;

out vec4 out_Color;

void main() {
  out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
}
