$input v_texcoord0

/*
    File: dhn_final.fs
    Author: Arves100
    Description:
        This fragmentation shader is computed at the final view step.

		Replaces all the d3d9 light and effects code with a single shader.
*/
#include "common.sh"  /* always include ! */

SAMPLER2D(s_viewtex, 0);

uniform vec4 u_light_ambient;
uniform vec4 u_light_diffuse;
uniform vec4 u_light_direction;

/*void calcLight()
{
	// D3DLIGHT9 from DirectGraphics...
	vec4 pixel = mul(u_light_diffuse, dot(u_light_direction, normal)) + u_light_ambient;
}*/

void main()
{
	gl_FragColor.w = 0.0;
	gl_FragColor.xyz = vec3(0.0, 0.0, 0.0);
}
