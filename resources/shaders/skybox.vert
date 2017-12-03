#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 2) in vec2 in_Texture_Coordinates;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec2 texture_Coordinates;

void main(void)
{
	gl_Position = (ProjectionMatrix * ViewMatrix) * vec4(in_Position, 1.0);
	texture_Coordinates = in_Texture_Coordinates;
}