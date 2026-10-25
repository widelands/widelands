#ifndef __EMSCRIPTEN__
#define H(x)x
H(#)version 120
#else
precision highp float;
#endif

varying vec3 var_color;

void main() {
	gl_FragColor = vec4(var_color, .8);
}
