#define H(x)x
#ifndef __EMSCRIPTEN__
H(#)version 120
#endif

varying vec4 var_color;

void main() {
	gl_FragColor = var_color;
}
