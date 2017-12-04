#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec4 in_Position;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 view_Direction;

void main(void)
{
	mat4 inverse_Projection_Matrix = inverse(ProjectionMatrix);
	mat3 transposed_View_Matrix = transpose(mat3(ViewMatrix));
	vec3 unprojected = (inverse_Projection_Matrix * in_Position).xyz;
	view_Direction = transposed_View_Matrix * unprojected;
	gl_Position = in_Position;
}
