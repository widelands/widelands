#version 120

uniform sampler2D u_terrain_texture;
uniform vec2 u_texture_dimensions;

varying float var_brightness;
varying vec2 var_texture_position;
varying vec2 var_texture_offset;

// TODO(sirver): This is a hack to make sure we are sampling inside of the
// terrain texture. This is a common problem with OpenGL and texture atlases.
#define MARGIN 1e-2

void main() {
	// The arbitrary multiplication by 0.99 makes sure that we never sample
	// outside of the texture in the texture atlas - this means non-perfect
	// pixel mapping of textures to the screen, but we are pretty meh about that
	// here.
	vec2 texture_fract = clamp(
			fract(var_texture_position),
			vec2(MARGIN, MARGIN),
			vec2(1. - MARGIN, 1. - MARGIN));
	vec4 clr = texture2D(u_terrain_texture, var_texture_offset + u_texture_dimensions * texture_fract);
	clr.rgb *= var_brightness;
	gl_FragColor = clr;
}
