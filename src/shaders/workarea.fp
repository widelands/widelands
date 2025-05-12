#define H(x)x
#ifndef __EMSCRIPTEN__
H(#)version 120
#endif

varying vec4 var_overlay;

void main() {
	gl_FragColor = var_overlay;
}
