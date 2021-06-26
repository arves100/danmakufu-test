$input v_texcoord1, v_color0

/*
	TLX RenderObject Fragmentation Shader for bgfx
*/

#include "common.sh"

SAMPLER2D(s_texDiffuse, 0);
SAMPLER2D(s_texNormal, 1);

void main()
{
	gl_FragColor = v_color0 * texture2D(s_texDiffuse, v_texcoord1);
}
