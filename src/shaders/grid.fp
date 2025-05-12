#define H(x)x
#ifndef __EMSCRIPTEN__
H(#)version 120
#endif

varying vec3 var_color;

void main() {
	gl_FragColor = vec4(var_color, .8);
}
