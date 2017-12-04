#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec4 in_Position;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 eyeDir;

void main(void)
{
	mat4 inverseProjection = inverse(ProjectionMatrix);
	mat3 inverseModelview = transpose(mat3(ViewMatrix));
	vec3 unprojected = (inverseProjection * in_Position).xyz;
	eyeDir = inverseModelview * unprojected;
	gl_Position = in_Position;

	// gl_Position = (ProjectionMatrix * ViewMatrix) * vec4(in_Position, 1.0);
	// texture_Coordinates = in_Position;
}
