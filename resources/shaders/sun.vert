#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_Texture_Coordinates;

// Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

uniform vec3 ColorVector;
uniform int ShaderMode;

out vec3 pass_Normal;
out vec3 vertex_Position;
out vec3 sun_Color;
out vec2 texture_Coordinates;
flat out int shader_Mode;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
	pass_Normal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;

	vec4 vertex_Position4 = ViewMatrix * ModelMatrix * vec4(in_Position, 1.0);
	vertex_Position = vertex_Position4.xyz / vertex_Position4.w;
	// vertex_Position_World = (ViewMatrix * vec4(vertex_Position, 1.0)).xyz;

	// transfer user input
	shader_Mode = ShaderMode;
	sun_Color = ColorVector;
	texture_Coordinates = in_Texture_Coordinates;
}
