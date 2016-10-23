#version 130

uniform sampler2D u_terrain_texture;
uniform sampler2D u_dither_texture;

uniform vec2 u_texture_dimensions;

in vec4 var_texture;
in vec4 var_dither[3];
in float var_brightness;

// TODO(sirver): This is a hack to make sure we are sampling inside of the
// terrain texture. This is a common problem with OpenGL and texture atlases.
#define MARGIN 1e-2

vec3 sample_terrain(vec2 offset, vec2 texcoord) {
	// The arbitrary multiplication by 0.99 makes sure that we never sample
	// outside of the texture in the texture atlas - this means non-perfect
	// pixel mapping of textures to the screen, but we are pretty meh about that
	// here.
	vec2 texture_fract = clamp(
			fract(texcoord),
			vec2(MARGIN, MARGIN),
			vec2(1. - MARGIN, 1. - MARGIN));
	return texture2D(u_terrain_texture, offset + u_texture_dimensions * texture_fract).rgb;
}

vec3 apply_dither(vec3 color, vec2 texcoord, vec4 dither) {
	vec2 offset = dither.xy;
	vec2 dither_tc = dither.zw;

	// Cut-off value to avoid unnecessary texture samples. The cut-off value
	// is chosen based on the dither mask, which happens to be 1 in 3/4 of the
	// texture.
	//
	// Note that cuting off in this way would be slightly incorrect if mipmaps
	// were used, because derivatives become undefined under non-uniform
	// control flow.
	if (dither_tc.y < 0.75)
		return color;

	float dither_factor = texture2D(u_dither_texture, dither_tc).a;
	vec3 dither_color = sample_terrain(offset, texcoord);

	// dither_factor is 0 when the other texture replace the base texture
	// entirely, and 1 when the base texture is not replaced at all.
	return mix(dither_color, color, dither_factor);
}

void main() {
	vec2 texcoord = var_texture.zw;
	vec3 clr = sample_terrain(var_texture.xy, texcoord);
	clr = apply_dither(clr, texcoord, var_dither[0]);
	clr = apply_dither(clr, texcoord, var_dither[1]);
	clr = apply_dither(clr, texcoord, var_dither[2]);
	clr *= var_brightness;
	gl_FragColor = vec4(clr, 1.);
}
