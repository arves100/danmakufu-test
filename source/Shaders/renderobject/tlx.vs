$input a_position, a_texcoord1, a_color0
$output v_texcoord1, v_color0

/*
	TLX RenderObject Vertex Shader for bgfx
*/

#include "common.sh"

uniform mat4 u_camera_matrix;

void main()
{
	gl_Position = mul(a_position, u_camera_matrix); // as specified in RenderObject.cpp
	v_texcoord1 = a_texcoord1;
	v_color0 = a_color0;
}
