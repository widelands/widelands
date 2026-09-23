#ifndef __EMSCRIPTEN__
#define H(x)x
H(#)version 120
#else
precision highp float;
#endif

// Inputs.
varying vec2 out_texture_position;
varying float out_brightness;

uniform sampler2D u_texture;

void main() {
	vec4 color = texture2D(u_texture, out_texture_position);
	color.rgb *= out_brightness;
	gl_FragColor = color;
}
