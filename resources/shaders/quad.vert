#version 150
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoord;

uniform int ShaderMode;

out vec2 texture_Coordinates;
flat out int shader_Mode;

void main(void) {
	gl_Position = vec4(in_Position, 0.0, 1.0);

	shader_Mode = ShaderMode;
	texture_Coordinates = in_TexCoord;
}
