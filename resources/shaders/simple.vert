#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_Texture_Coordinates;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

uniform sampler2D ColorTex;
uniform vec3 ColorVector;

uniform int ShaderMode;

out vec3 pass_Normal;
out vec3 pass_Normal_View;
out vec3 vertex_Position;
out vec3 vertex_Position_World;
out vec4 planet_Color;
flat out int shader_Mode;


void main(void)
{
	gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
	pass_Normal = (ModelMatrix * vec4(in_Normal, 0.0)).xyz;
	pass_Normal_View = (ViewMatrix * vec4(pass_Normal, 0.0)).xyz;

	vec4 vertex_Position4 = ModelMatrix * vec4(in_Position, 1.0);
	vertex_Position = vertex_Position4.xyz / vertex_Position4.w;
	vertex_Position_World = (ViewMatrix * vec4(vertex_Position, 1.0)).xyz;

	// transfer user input
	// planet_Color = ColorVector;
	planet_Color = texture(ColorTex, in_Texture_Coordinates);
	shader_Mode = ShaderMode;
}
