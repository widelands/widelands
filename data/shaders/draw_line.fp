#version 120

varying vec3 var_color;

void main() {
	gl_FragColor = vec4(var_color.rgb, 1.);
}
