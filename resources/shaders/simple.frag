#version 150

in vec3 pass_Normal;
in vec3 pass_Normal_View;
in vec3 vertex_Position;
in vec3 vertex_Position_World;
in vec3 planet_Color;
in vec2 texture_Coordinates;

uniform sampler2D ColorTex;

flat in int shader_Mode;

out vec4 out_Color;

// switch between viewing-modi

const vec3 light_Position = vec3(0.0, 0.0, 0.0);
const vec3 specular_Color = vec3(1.0, 1.0, 1.0); // color of the specular highlights
const vec3 ambient_Color = vec3(0.01, 0.01, 0.01);  // indirect light coming from sourroundings
const vec3 diffuse_Color = vec3(0.5, 0.5, 0.5);  // diffusely reflected light from surface microfacets
// const float sun_Intensity = 1.0;
// const float ambient_Intensity = 0.01;
// const float diffuse_Intensity = 0.5;
// const float specular_Intensity = 1.0;
const float shininess = 16.0;
const float screen_Gamma = 2.2;

void main() {
  // Blinn-Phong-Model
  vec3 N  = normalize(pass_Normal); // normal
  vec3 NV = normalize(pass_Normal_View); // normal view
  vec3 L  = normalize(light_Position - vertex_Position); // light direction
  vec3 V  = normalize(-vertex_Position); // view direction
  vec3 H  = normalize(L + V); // halfway vector

  float lambertian = max(dot(L, N), 0.0); // diffuse reflectance
  float specular = 0.0; // reflection of light directly to viewer
  float specular_Angle = max(dot(H, N), 0.0); // rho

  vec3 color_Linear;

  // calculate specular reflection if the surface is oriented to the light source
  if(lambertian > 0.0) {
    specular = pow(specular_Angle, shininess * 10);
  }

  // calculate planet color
  // color_Linear = ambient_Color + lambertian * diffuse_Color * vec3(planet_Color).xyz + specular * specular_Color;
  color_Linear = ambient_Color + lambertian * diffuse_Color * texture(ColorTex, texture_Coordinates).xyz + specular * specular_Color;

  // Cell-Shading-Model
  if (shader_Mode == 2) {
    float normal_View_Angle = dot(-normalize(vertex_Position_World), NV);

    specular = pow(specular_Angle, shininess * 10);

    if(lambertian > 0.9) {lambertian = 1;}
    else if(lambertian > 0.6) {lambertian = 0.9;}
    else if(lambertian > 0.3) {lambertian = 0.6;}
    else if(lambertian > 0.0) {lambertian = 0.3;}

    // highlight the planets border in a different color
    if(abs(normal_View_Angle) < 0.3) {
      color_Linear = vec3(1.0, 0.0, 0.0);
    } else {
      // calculate planet color
      color_Linear = ambient_Color + lambertian * diffuse_Color * vec3(planet_Color).xyz + specular * specular_Color;
    }
  }

  // calculate gamme correction
  vec3 color_Gamma_Corrected = pow(color_Linear, vec3(1.0/screen_Gamma));
  out_Color = vec4(color_Gamma_Corrected, 1.0);
}
