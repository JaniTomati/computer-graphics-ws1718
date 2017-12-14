#version 150

in vec2 texture_Coordinates;
flat in int shader_Mode;

uniform sampler2D FramebufferTex;

out vec4 out_Color;

void main() {

  float tex_x = texture_Coordinates.x;
  float tex_y = texture_Coordinates.y;

  out_Color = texture(FramebufferTex, texture_Coordinates);

  if(shader_Mode == 7) {
    // greyscale
    float avg = 0.2126 * out_Color.r + 0.7152 * out_Color.g + 0.0722 * out_Color.b;
    out_Color = vec4(avg, avg, avg, 1.0);
  }else if (shader_Mode == 8) {
    // horizontal
		tex_y = 1 - tex_y;
		out_Color = texture(FramebufferTex, vec2(tex_x, tex_y));
  }else if (shader_Mode == 9) {
    // vertical
    tex_x = 1 - tex_x;
		out_Color = texture(FramebufferTex, vec2(tex_x, tex_y));
  }else if (shader_Mode == 0) {
    // blur
    const float blur_x = 1.0 / 500.0;
    const float blur_y = 1.0 / 500.0;
    vec4 sum = vec4(0.0);
    for (int x = -3; x <= 3; x++) {
      for (int y = -3; y <= 3; y++) {
        sum += texture(FramebufferTex, vec2(tex_x + x * blur_x,
          tex_y + y * blur_y)) / 81.0;
          out_Color = sum;
        }
      }
  } else {
    out_Color = texture(FramebufferTex, texture_Coordinates);
  }
}
