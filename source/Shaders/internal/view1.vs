$input a_position, a_texcoord0
$output v_texcoord0

/*
    File: dhn_final.vs
    Author: Arves100
    Description:
        This vertex shader is computed at the final view step.

        The render only accepts a rectangle that contains the framebuffer here
*/
#include "common.sh" /* always include ! */

void main()
{
    v_texcoord0 = a_texcoord0;
    gl_Position.xyz = a_position;
    gl_Position.w = 0.0;
}
