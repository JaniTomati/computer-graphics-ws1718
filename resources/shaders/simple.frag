#version 150

in vec3 pass_Normal;
in vec3 vertex_Position;
in vec3 vertex_Position_World;
in vec3 planet_Color;

out vec4 out_Color;

// switch between viewing-modi
int mode;

const vec3 light_Position = vec3(0.0, 0.0, 0.0);
const vec3 specular_Color = vec3(1.0, 1.0, 1.0); // color of the specular highlights
const vec3 ambient_Color = vec3(0.5, 0.0, 0.0);  // indirect light coming from sourroundings
const vec3 diffuse_Color = vec3(0.5, 0.0, 0.0);  // diffusely reflected light from surface microfacets
const float shininess = 16.0;
const float screen_Gamma = 2.2;

void main() {

  int mode = 1;

  vec3 N = normalize(pass_Normal); // normal
  vec3 L = normalize(light_Position - vertex_Position); // light direction
  vec3 V = normalize(-vertex_Position); // view direction
  vec3 H = normalize(L + V); // halfway vector

  float lambertian = max(dot(L, N), 0.0); // diffuse reflectance
  float specular = 0.0; // reflection of light directly to viewer

  // calculate specular reflection if the surface is oriented to the light source
  if(lambertian > 0.0) {
    float specular_Angle = max(dot(H, N), 0.0); // rho
    specular = pow(specular_Angle, shininess);
  }

  if(mode == 1) {
    vec3 color_Linear = ambient_Color + lambertian * diffuse_Color + specular * specular_Color;

    vec3 color_Gamma_Corrected = pow(color_Linear, vec3(1.0/screen_Gamma));
    out_Color = vec4(color_Gamma_Corrected, 1.0);

  } else if (mode == 2) {
    float normal_View_Angle = dot(-normalize(vertex_Position_World), N);
    vec3 color_Linear;

    if(lambertian > 0.8) {lambertian = 1;}
    else if(lambertian > 0.6) {lambertian = 0.8;}
    else if(lambertian > 0.4) {lambertian = 0.6;}
    else if(lambertian > 0.2) {lambertian = 0.4;}
    else if(lambertian > 0.0) {lambertian = 0.2;}

    if(abs(normal_View_Angle) < 0.5) {
      color_Linear = vec3(1.0, 1.0, 0.0);
    } else {
      vec3 color_Linear = ambient_Color + lambertian * diffuse_Color + specular * specular_Color;
    }

    vec3 color_Gamma_Corrected = pow(color_Linear, vec3(1.0/screen_Gamma));
    out_Color = vec4(color_Gamma_Corrected, 1.0);
  }
}
