precision highp float;

varying vec3 var_color;

void main() {
	gl_FragColor = vec4(var_color, .8);
}
