#version 120

uniform sampler2D u_texture;
uniform sampler2D u_mask;

varying vec4 out_texture_coordinates;
varying vec4 out_blend;
varying float out_program_flavor;

void main() {
	vec4 texture_color = texture2D(u_texture, out_texture_coordinates.xy);

	// See http://en.wikipedia.org/wiki/YUV.
	float luminance = dot(vec3(0.299, 0.587, 0.114), texture_color.rgb);

	if (out_program_flavor == 0.) {
		gl_FragColor = vec4(texture_color.rgb, out_blend.a * texture_color.a);
	} else if (out_program_flavor == 1.) {
		gl_FragColor = vec4(vec3(luminance) * out_blend.rgb, out_blend.a * texture_color.a);
	} else {
		vec4 mask_color = texture2D(u_mask, out_texture_coordinates.zw);
		float blend_influence = mask_color.r * mask_color.a;
		gl_FragColor = vec4(
			mix(texture_color.rgb, out_blend.rgb * luminance, blend_influence),
				out_blend.a * texture_color.a);
	}
}
