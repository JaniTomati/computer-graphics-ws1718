#version 150

in vec3 pass_Normal;
in vec3 vertex_Position;
in vec3 planet_Color;

out vec4 out_Color;

const vec3 light_Position = vec3(0.0, 0.0, 0.0);
const vec3 specular_Color = vec3(1.0, 1.0, 1.0); // color of the specular highlights
const vec3 ambient_Color = vec3(0.5, 0.0, 0.0);  // indirect light coming from sourroundings
const vec3 diffuse_Color = vec3(0.5, 0.0, 0.0);  // diffusely reflected light from surface microfacets
const float shininess = 16.0;
const float screen_Gamma = 2.2;

void main() {
  vec3 N = normalize(pass_Normal);
  vec3 L = normalize(light_Position - vertex_Position); // light direction
  vec3 V = normalize(-vertex_Position); // view direction

  float lambertian = max(dot(N, L), 0.0); // diffuse reflectance

  float specular = 0.0; // reflection of light directly to viewer
  // calculate specular reflection if the surface is oriented to the light source
  if(lambertian > 0.0) {
    vec3 H = normalize(L + V); // halfway vector
    float specular_Angle = max(dot(H, N), 0.0); // rho
    specular = pow(specular_Angle, shininess);
   }

   vec3 color_Linear = ambient_Color + lambertian * diffuse_Color + specular * specular_Color;

  vec3 color_Gamma_Corrected = pow(color_Linear, vec3(1.0/screen_Gamma));
  out_Color = vec4(color_Gamma_Corrected, 1.0);
}
