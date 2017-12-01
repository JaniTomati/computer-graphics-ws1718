#version 150

in vec3 pass_Normal_View;
in vec3 vertex_Position_World;
in vec3 sun_Color;
in vec2 texture_Coordinates;

uniform sampler2D ColorTex;
flat in int shader_Mode;

out vec4 out_Color;

void main() {
  // Cell Shading Model
  if (shader_Mode == 2) {
    vec3 NV = normalize(pass_Normal_View); // normal view
    vec3 color_Linear;
    float normal_View_Angle = dot(-normalize(vertex_Position_World), NV);

    // highlight suns border in a different color
    if(abs(normal_View_Angle) < 0.1) {
      color_Linear = vec3(1.0, 0.0, 0.0);
    } else {
      // set the sun color
      color_Linear = vec3(sun_Color).xyz;
    }
    out_Color = vec4(color_Linear, 1.0);

    // default: set plain sun color
  } else {
    //out_Color = vec4(vec3(sun_Color).xyz, 1.0);
    out_Color = texture(ColorTex, texture_Coordinates);
  }
}
