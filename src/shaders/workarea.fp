#ifndef __EMSCRIPTEN__
#define H(x)x
H(#)version 120
#else
precision highp float;
#endif

varying vec4 var_overlay;

void main() {
	gl_FragColor = var_overlay;
}
