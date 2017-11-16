#version 150

in vec3 pass_Normal;
in vec3 camera_Position;
in vec3 planet_Color;
in vec3 world_Position;
in vec3 world_Normal;

out vec4 out_Color;

const vec3 light_Position = vec3(0.0, 0.0, 0.0);
const vec3 specular_Color = vec3(1.0, 1.0, 1.0);
const vec3 ambient_Color;
const vec3 diffuse_Color;
const float shininess;

void main() {
  vec3 N = normalize(world_Normal);
  vec3 L = normalize(light_Position - world_Position);

  float lambertian = max(dot(L, N), 0.0);
  float specular = 0.0;

  out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
}
