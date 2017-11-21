#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;

uniform vec3 ColorVector;

out vec3 pass_Normal;
out vec3 vertex_Position;
out vec3 vertex_Position_World;
out vec3 planet_Color;


void main(void)
{
	gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
	pass_Normal = (ModelMatrix * vec4(in_Normal, 0.0)).xyz;

	vec4 vertex_Position4 = ModelMatrix * vec4(in_Position, 1.0);
	vertex_Position = vertex_Position4.xyz / vertex_Position4.w;
	vertex_Position_World = (ViewMatrix * vec4(vertex_Position, 1.0)).xyz;

	planet_Color = ColorVector;
}
