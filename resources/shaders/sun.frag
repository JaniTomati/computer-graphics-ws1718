#version 150

in vec3 pass_Normal_View;
in vec3 vertex_Position_World;
in vec3 sun_Color;
flat in int shader_Mode;

out vec4 out_Color;

void main() {

  if (shader_Mode == 2) {
    vec3 NV = normalize(pass_Normal_View); // normal view
    vec3 color_Linear;
    float normal_View_Angle = dot(-normalize(vertex_Position_World), NV);

    if(abs(normal_View_Angle) < 0.1) {
      color_Linear = vec3(1.0, 0.0, 0.0);
    } else {
      color_Linear = sun_Color;
    }
    out_Color = vec4(color_Linear, 1.0);

  } else {
    out_Color = vec4(sun_Color, 1.0);
  }
}
