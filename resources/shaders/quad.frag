#version 150

in vec2 texture_Coordinates;
// flat in int shader_Mode;

uniform sampler2D FramebufferTex;

uniform bool HorizontalReflectionMode;
uniform bool GreyScaleMode;
uniform bool VerticalReflectionMode;
uniform bool BlurMode;
uniform bool GodRays;
uniform vec4 LightCenter;
uniform sampler2D LightTex;

in vec4 gl_FragCoord;

out vec4 out_Color;

const float num_Samples = 200.0;


void main() {

  float tex_x = texture_Coordinates.x;
  float tex_y = texture_Coordinates.y;

  out_Color = texture(FramebufferTex, texture_Coordinates);

  if (GodRays) {
    float exposure = 0.75;
    float decay = 0.97;
    float density = 0.5;
    float weight = 0.1;
    float illumination_Decay = 0.5;
    vec2 tex_Coord = texture_Coordinates.xy;

   vec2 light_Pos = (LightCenter.xy + vec2(1.0, 1.0)) / 2;
   vec2 updated_tex_Coord = (tex_Coord - light_Pos) / num_Samples * density;

   vec4 final_Light = texture2D(LightTex, tex_Coord);

   for (int i = 0; i < num_Samples; ++i) {
     tex_Coord -= updated_tex_Coord;
     vec4 sample_Light = texture2D(LightTex, tex_Coord);
     sample_Light *= illumination_Decay * weight;
     final_Light += sample_Light;
     illumination_Decay *= decay;
   }
   out_Color = final_Light * exposure;
  }

  if (HorizontalReflectionMode) {
    // horizontal
		tex_y = 1 - tex_y;
		out_Color = texture(FramebufferTex, vec2(tex_x, tex_y));
  }
  if (VerticalReflectionMode) {
    // vertical
    tex_x = 1 - tex_x;
		out_Color = texture(FramebufferTex, vec2(tex_x, tex_y));
  }
  if (BlurMode) {
    // blur
    float temp_x = 1.0 / 1024;
    float temp_y = 1.0 / 768;
    // float temp_x = 1.0 / gl_FragCoord.x;
    // float temp_y = 1.0 / gl_FragCoord.y;
    vec4 sum = vec4(0.0);
    vec2 temp = vec2(0.0);
    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        // temp_x_y entfernen für grosses Kino (geht grade nicht -.-)
        temp = vec2(tex_x + x * temp_x, tex_y + y * temp_y);
        if(x == 1 && y == 1) {
          sum += texture(FramebufferTex, temp) * 	0.195346;
        } else if(mod(x, 2) == 0 || mod(y, 2) == 0) {
          sum += texture(FramebufferTex, temp) * 	0.123317;
        } else {
          sum += texture(FramebufferTex, temp) * 	0.077847;
        }
      }
    }
    out_Color = sum;
  }
  if(GreyScaleMode) {
    // greyscale
    float avg = 0.2126 * out_Color.r + 0.7152 * out_Color.g + 0.0722 * out_Color.b;
    out_Color = vec4(avg, avg, avg, 1.0);
  }
}
