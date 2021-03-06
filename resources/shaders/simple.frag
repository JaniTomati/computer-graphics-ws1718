#version 150

in vec3 pass_Normal;
in vec3 pass_Normal_View; //opt
in vec3 pass_Tangent;
in vec3 vertex_Position;
in vec3 vertex_Position_View;
in vec3 planet_Color;
in vec2 texture_Coordinates;
in vec3 vertex_Position_Cam;
in vec3 light_Position;

uniform sampler2D ColorTex;
uniform sampler2D NormalTex;

flat in int shader_Mode;

out vec4 out_Color;

// switch between viewing-modi

// const vec3 light_Position = vec3(0.0, 0.0, 0.0);
const vec3 specular_Color = vec3(1.0, 1.0, 1.0); // color of the specular highlights
// const vec3 ambient_Color = vec3(0.01, 0.01, 0.01);  // indirect light coming from sourroundings
vec3 ambient_Color = vec3(texture(ColorTex, texture_Coordinates)).xyz * 0.01;  // indirect light coming from sourroundings
// const vec3 diffuse_Color = vec3(0.5, 0.5, 0.5);  // diffusely reflected light from surface microfacets
vec3 diffuse_Color = vec3(texture(ColorTex, texture_Coordinates)).xyz;  // diffusely reflected light from surface microfacets
// const float sun_Intensity = 1.0;
// const float ambient_Intensity = 0.01;
// const float diffuse_Intensity = 0.5;
// const float specular_Intensity = 1.0;
const float shininess = 16.0;
const float screen_Gamma = 2.2;
vec3 normal_Mapping = 2 * texture(NormalTex, texture_Coordinates).rgb - 1.0f;

void main() {
  // normal mapping
  vec3 bi_Tangent = cross(pass_Normal, pass_Tangent);
  mat3 TangentMatrix = mat3(pass_Tangent, bi_Tangent, pass_Normal);
  vec3 normal = normalize(TangentMatrix * normal_Mapping); // detail normal
  // vec3 N  = normalize(normal); // normal
  // vec3 NV = normalize(pass_Normal_View); // normal view
  vec3 L  = normalize(light_Position - vertex_Position); // light direction
  vec3 V  = normalize(-vertex_Position); // view direction


  // Blinn-Phong-Model
  float lambertian = max(dot(L, normal), 0.0); // diffuse reflectance

  vec3 H  = normalize(L + V); // halfway vector

  float specular_Angle = max(dot(H, normal), 0.0); // rho
  float specular = 0.0; // reflection of light directly to viewer


  // calculate specular reflection if the surface is oriented to the light source
  if(lambertian > 0.0) {
    specular = pow(specular_Angle, shininess * 10);
  }

  // calculate planet color
  // color_Linear = ambient_Color + lambertian * diffuse_Color * vec3(planet_Color).xyz + specular * specular_Color;
  vec3 color_Linear = ambient_Color + lambertian * diffuse_Color + specular * specular_Color;

  // Cell-Shading-Model
  if (shader_Mode == 2) {

    float normal_View_Angle = dot(-normalize(vertex_Position), normalize(pass_Normal));

    diffuse_Color = planet_Color;
    ambient_Color = vec3(0.01, 0.01, 0.01);

    specular_Angle = max(dot(H, normalize(pass_Normal)), 0.0);
    specular = pow(specular_Angle, shininess * 10);
    lambertian = max(dot(L, normalize(pass_Normal)), 0.0);

    if(lambertian > 0.9) {lambertian = 1;}
    else if(lambertian > 0.6) {lambertian = 0.9;}
    else if(lambertian > 0.3) {lambertian = 0.6;}
    else if(lambertian > 0.0) {lambertian = 0.3;}

    // highlight the planets border in a different color
    if(abs(normal_View_Angle) < 0.3) {
      color_Linear = vec3(1.0, 0.0, 0.0);
    } else {
      // calculate planet color
      color_Linear = ambient_Color + lambertian * diffuse_Color + specular * specular_Color;
    }
  }

  // calculate gamme correction
  vec3 color_Gamma_Corrected = pow(color_Linear, vec3(1.0/screen_Gamma));
  out_Color = vec4(color_Gamma_Corrected, 1.0);
}
